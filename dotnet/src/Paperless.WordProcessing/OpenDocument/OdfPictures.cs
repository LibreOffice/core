using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.Core.Graphics;
using Paperless.OpenDocument;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// Finds the bytes behind a <c>draw:image</c>, which ODF states in one of two mutually exclusive ways.
/// </summary>
/// <remarks>
/// <para>
/// A packaged document names its picture with <c>xlink:href</c>, pointing at an entry of the package —
/// conventionally under <c>Pictures/</c>, though the name is a producer's choice. A flat XML document
/// has no package to point into, so it carries the bytes as base64 in an <c>office:binary-data</c>
/// child instead.
/// </para>
/// <para>
/// <strong>The href wins, and its absence is what enables the other.</strong> Not a preference — a rule,
/// and one worth stating because the obvious reading of it is backwards.
/// <c>XMLTextFrameContext_Impl::createFastChildContext</c>
/// (<c>xmloff/source/text/XMLTextFrameContext.cxx:1139</c>) only opens a base64 stream when no URL has
/// been seen, and <c>Create()</c> at line 667 takes <c>loadGraphicByURL(sHRef)</c> whenever the href is
/// non-empty. So a <c>data:</c> URI in <c>xlink:href</c> is <em>accepted and then silently ignored</em>:
/// the href is not empty, so the <c>office:binary-data</c> beside it is never read, and the URI itself
/// resolves to nothing. This reader reproduces that rather than repairing it, because a hand-written
/// fixture using both would otherwise render here and not in LibreOffice — which is the one difference a
/// comparison cannot see, since both produce a picture and only one of them is the reference.
/// </para>
/// </remarks>
public sealed class OdfPictures
{
    private readonly OdfFile? _file;
    private readonly List<Diagnostic>? _diagnostics;

    /// <summary>Creates a resolver over a document.</summary>
    /// <param name="file">
    /// The open document, for reaching a package entry an href names. Null for a caller with no package
    /// to reach into, which leaves only the inline base64 form readable — the flat XML case.
    /// </param>
    /// <param name="diagnostics">Where to record a picture that will not draw, or null to say nothing.</param>
    public OdfPictures(OdfFile? file, List<Diagnostic>? diagnostics)
    {
        _file = file;
        _diagnostics = diagnostics;
    }

    /// <summary>
    /// The picture a <c>draw:frame</c> holds, or nothing when it holds none this can draw.
    /// </summary>
    /// <remarks>
    /// The first <c>draw:image</c> child only. A <c>draw:frame</c> may carry several children as
    /// alternatives in decreasing order of preference — a <c>draw:object</c> and then a
    /// <c>draw:image</c> of it as a fallback is how ODF stores a chart — and the first one that can be
    /// drawn is the one LibreOffice takes.
    /// </remarks>
    public FramePicture Read(XElement frame)
    {
        ArgumentNullException.ThrowIfNull(frame);

        XElement? image = frame.Element(XName.Get("image", OdfNamespaces.Draw));
        if (image is null) return FramePicture.None;

        string? mediaType = image.Attribute(XName.Get("mime-type", OdfNamespaces.Draw))?.Value;
        string? href = image.Attribute(XName.Get("href", OdfNamespaces.XLink))?.Value;

        if (!string.IsNullOrEmpty(href))
        {
            return FromPackage(href, mediaType);
        }

        XElement? data = image.Element(XName.Get("binary-data", OdfNamespaces.Office));
        if (data is null) return FramePicture.None;

        byte[]? bytes = Base64(data.Value);
        if (bytes is null)
        {
            _diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2372",
                "A picture's office:binary-data is not valid base64, so the frame stays empty."));
            return FramePicture.None;
        }

        return EmbeddedPicture.Read(bytes, mediaType, "office:binary-data", _diagnostics);
    }

    /// <summary>
    /// The bytes of a package entry an href names, or nothing when the package has no such entry.
    /// </summary>
    /// <remarks>
    /// A linked picture — one whose href is a <c>file:</c> or <c>http:</c> URL rather than a package
    /// entry — is deliberately not fetched. Reading a document must not reach the network or the file
    /// system beside it, and LibreOffice's own behaviour for one that cannot be reached is the same
    /// empty frame.
    /// </remarks>
    private FramePicture FromPackage(string href, string? mediaType)
    {
        if (_file is null) return FramePicture.None;

        string entry = href.StartsWith("./", StringComparison.Ordinal) ? href[2..] : href;
        if (entry.Length == 0 || entry.Contains("://", StringComparison.Ordinal)) return FramePicture.None;

        using Stream? part = _file.OpenPart(entry);
        if (part is null)
        {
            _diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2373",
                $"A picture names the package entry '{entry}', which the package does not hold, "
                + "so the frame stays empty.",
                new DiagnosticLocation(entry)));
            return FramePicture.None;
        }

        using MemoryStream buffer = new();
        part.CopyTo(buffer);

        return EmbeddedPicture.Read(buffer.ToArray(), mediaType, entry, _diagnostics);
    }

    /// <summary>
    /// Decodes base64 that may be laid out across lines, or null when it is not base64 at all.
    /// </summary>
    /// <remarks>
    /// <c>Convert.FromBase64String</c> already tolerates the white space an XML pretty-printer inserts,
    /// so the only thing left to do is to turn its exception into the null this library's leniency rule
    /// wants. A picture that will not decode is a document defect, not a reason to fail the read.
    /// </remarks>
    private static byte[]? Base64(string text)
    {
        try
        {
            return Convert.FromBase64String(text);
        }
        catch (FormatException)
        {
            return null;
        }
    }
}
