using System.Globalization;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>One rendered page's sheet size, in points.</summary>
/// <param name="Width">Its width.</param>
/// <param name="Height">Its height.</param>
public readonly record struct PdfPageSize(double Width, double Height);

/// <summary>
/// Reads each page's sheet size out of a PDF.
/// </summary>
/// <remarks>
/// <para>
/// The one thing about a rendered page that needs no interpretation at all: a page's size is stated in the
/// document, carried into the PDF unchanged, and reported by <c>pdftotext -bbox</c> in its own attributes.
/// So it is the cheapest possible check that a section's geometry took effect — a reader that carried the
/// first section's setup throughout gets three portrait pages where the reference gives two portrait and a
/// landscape, and no amount of comparing word positions says so as plainly.
/// </para>
/// <para>
/// Parsed from the <c>-bbox</c> output rather than from the PDF's own <c>/MediaBox</c>, so that one
/// invocation of one tool serves both this and <see cref="PdfWords"/>.
/// </para>
/// </remarks>
public static partial class PdfPageSizes
{
    /// <summary>The sizes of a PDF's pages, in order, or nothing when the tool is unavailable.</summary>
    /// <param name="pdf">The PDF to read.</param>
    public static List<PdfPageSize> Read(string? pdf)
    {
        List<PdfPageSize> sizes = [];
        if (string.IsNullOrEmpty(pdf) || !File.Exists(pdf)) return sizes;

        string? bbox = PdfWords.RunBoundingBoxes(pdf);
        if (bbox is null) return sizes;

        foreach (Match match in PageAttributes().Matches(bbox))
        {
            if (double.TryParse(
                    match.Groups["w"].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                    out double width)
                && double.TryParse(
                    match.Groups["h"].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                    out double height))
            {
                sizes.Add(new PdfPageSize(width, height));
            }
        }

        return sizes;
    }

    [GeneratedRegex(
        """<page\s+width="(?<w>[-0-9.eE]+)"\s+height="(?<h>[-0-9.eE]+)"\s*>""",
        RegexOptions.CultureInvariant)]
    private static partial Regex PageAttributes();
}
