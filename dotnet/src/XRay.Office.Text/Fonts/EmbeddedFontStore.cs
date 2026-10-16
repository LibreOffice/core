using System.Collections.Concurrent;
using System.Security.Cryptography;
using XRay.Office.Core.Graphics;

namespace XRay.Office.Text.Fonts;

/// <summary>
/// Materialises a document's own embedded face so that a
/// <see cref="FontReference.FaceKey"/> naming it is a path every backend can open.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The key has to be a path, and that is the whole reason this type exists.</strong>
/// Resolution ends at a <see cref="FontReference"/>, and every consumer downstream of it reads
/// <c>FaceKey</c> as <c>path</c> or <c>path#index</c> and opens the file:
/// <c>SystemFontResolver.LoadFace</c>, <c>FileFontProvider</c> for PDF output, and
/// <c>SkiaDrawingSink.TypefaceFor</c> for raster output. A key those cannot open does not fail
/// loudly — the PDF <em>names</em> the face and embeds nothing, which the gate reads as
/// <c>unembedded</c> and a reader shows in a substituted face. That exact defect has been found
/// twice before in this tree, once on decks and once on spreadsheets; see the remarks on
/// <c>SlideTextLayout.Run</c>.
/// </para>
/// <para>
/// So the bytes are written out, which is also what LibreOffice does with them:
/// <c>EmbeddedFontsManager::addEmbeddedFont</c> ends in <c>writeFontBytesToFile</c> and registers
/// the resulting URL, for the same reason — the layer that draws takes a file.
/// </para>
/// <para>
/// Content-addressed and process-scoped. Content-addressed because a deck embeds one face and
/// draws with it on every slide, and because two decks in one batch run routinely embed the same
/// face; process-scoped because the alternatives are worse in both directions — a store shared
/// between processes cannot be cleaned up by any of them, and one scoped to a document is gone by
/// the time a caller that lays out, disposes and then renders reaches the renderer.
/// </para>
/// </remarks>
public static class EmbeddedFontStore
{
    private static readonly ConcurrentDictionary<string, string> Stored = new(StringComparer.Ordinal);
    private static readonly Lock Gate = new();
    private static string? _directory;

    /// <summary>
    /// Writes a face's bytes where they can be opened by path, and answers that path.
    /// </summary>
    /// <remarks>
    /// Null rather than an exception when the bytes cannot be written: a face that cannot be
    /// materialised costs the document a substitution, which is the ordinary outcome for a family
    /// nothing has, and not the document itself. That is the leniency rule, applied to the one
    /// step here that touches a filesystem.
    /// </remarks>
    /// <param name="font">The sfnt, already unwrapped from whatever container carried it.</param>
    /// <returns>A path usable as a <see cref="FontRequest.EmbeddedFaceKey"/>, or null.</returns>
    public static string? Store(ReadOnlySpan<byte> font)
    {
        if (font.Length == 0) return null;

        string name = Convert.ToHexStringLower(SHA256.HashData(font)) + ".ttf";
        if (Stored.TryGetValue(name, out string? existing)) return existing;

        byte[] bytes = font.ToArray();

        try
        {
            string directory = Directory();
            string path = Path.Combine(directory, name);

            // Written under a unique name and moved into place, so that two threads storing the
            // same face cannot leave a half-written file behind for the other to read as a font.
            if (!File.Exists(path))
            {
                string pending = path + "." + Environment.CurrentManagedThreadId.ToString(
                    System.Globalization.CultureInfo.InvariantCulture) + ".tmp";

                File.WriteAllBytes(pending, bytes);
                File.Move(pending, path, overwrite: true);
            }

            Stored[name] = path;
            return path;
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            return null;
        }
    }

    private static string Directory()
    {
        if (_directory is { } ready) return ready;

        lock (Gate)
        {
            if (_directory is { } raced) return raced;

            string path = Path.Combine(
                Path.GetTempPath(),
                "xray-office-embedded-fonts-" + Environment.ProcessId.ToString(
                    System.Globalization.CultureInfo.InvariantCulture));

            System.IO.Directory.CreateDirectory(path);
            AppDomain.CurrentDomain.ProcessExit += (_, _) => Discard(path);

            _directory = path;
            return path;
        }
    }

    /// <summary>
    /// Removes the store at exit, best effort.
    /// </summary>
    /// <remarks>
    /// Best effort and deliberately silent. The files are in the platform temporary directory and
    /// a few hundred kilobytes at most, so failing to remove them is untidy rather than harmful —
    /// and throwing out of a process-exit handler would turn a rendered document into a crash.
    /// </remarks>
    private static void Discard(string path)
    {
        try
        {
            System.IO.Directory.Delete(path, recursive: true);
        }
        catch (Exception exception) when (exception is IOException
                                             or UnauthorizedAccessException
                                             or DirectoryNotFoundException)
        {
            // Nothing useful to do at exit.
        }
    }
}
