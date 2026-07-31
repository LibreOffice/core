using System.Collections.Concurrent;
using Paperless.Core.Graphics;

namespace Paperless.Rendering.Pdf;

/// <summary>
/// Finds the bytes of a face a laid-out page asks to draw with.
/// </summary>
/// <remarks>
/// Separate from <c>Paperless.Text</c>'s <c>IFontResolver</c>, and deliberately so: by the
/// time a <see cref="GlyphRun"/> reaches a backend, resolution has already happened and
/// choosing a face again would be choosing a different one. All that is left is to fetch
/// the file the layout measured against, which is what
/// <see cref="FontReference.FaceKey"/> names.
/// </remarks>
public interface IPdfFontProvider
{
    /// <summary>The face's file, or null when it cannot be found.</summary>
    /// <remarks>
    /// Null is not an error. A PDF that names a font without embedding it still holds the
    /// right text at the right pen positions; only the glyph shapes fall back to whatever
    /// the reader has.
    /// </remarks>
    PdfFontProgram? Load(FontReference font);
}

/// <summary>A font file and which face of it to use.</summary>
/// <param name="Data">The whole file, collection and all.</param>
/// <param name="FaceIndex">Which face of a collection; zero for a single font.</param>
public readonly record struct PdfFontProgram(ReadOnlyMemory<byte> Data, int FaceIndex);

/// <summary>
/// Loads faces from the file system, reading <see cref="FontReference.FaceKey"/> as a path.
/// </summary>
/// <remarks>
/// <para>
/// The default provider, because the default resolver produces exactly that key:
/// <c>SystemFontResolver</c> builds it as <c>path</c>, or <c>path#index</c> for one face of
/// a collection (<c>InstalledFace.FaceKey</c>). A key that is not a path — which is what a
/// hand-built page or a document's own embedded font produces — simply does not load, and
/// the PDF names the family without embedding it.
/// </para>
/// <para>
/// Files are cached by key for the lifetime of the provider. A document setting eleven
/// point body text and nine point notes in one family asks for the same file on every
/// page, and a system font is megabytes.
/// </para>
/// </remarks>
public sealed class FileFontProvider : IPdfFontProvider
{
    /// <summary>A shared instance; the cache it holds is safe to use from several threads.</summary>
    public static FileFontProvider Instance { get; } = new();

    private readonly ConcurrentDictionary<string, PdfFontProgram?> _cache = new(StringComparer.Ordinal);

    /// <inheritdoc/>
    public PdfFontProgram? Load(FontReference font)
    {
        ArgumentNullException.ThrowIfNull(font);
        if (string.IsNullOrEmpty(font.FaceKey)) return null;

        return _cache.GetOrAdd(font.FaceKey, static key =>
        {
            (string path, int index) = Split(key);

            try
            {
                return File.Exists(path) ? new PdfFontProgram(File.ReadAllBytes(path), index) : null;
            }
            catch (IOException)
            {
                return null;
            }
            catch (UnauthorizedAccessException)
            {
                return null;
            }
        });
    }

    /// <summary>Splits <c>path#index</c>, which is how a collection's face is keyed.</summary>
    private static (string Path, int Index) Split(string key)
    {
        int hash = key.LastIndexOf('#');
        if (hash <= 0 || !int.TryParse(key[(hash + 1)..], out int index)) return (key, 0);

        return (key[..hash], index);
    }
}
