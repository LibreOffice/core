using System.Diagnostics;
using System.Globalization;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>One word as <c>pdftotext -bbox</c> reports it.</summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="Left">The left edge of its box, in points from the page's left edge.</param>
/// <param name="Right">The right edge.</param>
/// <param name="Top">
/// The top of its box, in points from the page's top — which is <em>not</em> a baseline: it sits above one
/// by the font's ascent, and the PDF never states that. Useful for comparing two words' vertical
/// positions, not for comparing one against a layout engine's baseline.
/// </param>
/// <param name="Text">The word.</param>
public readonly record struct PdfWord(
    int PageIndex,
    double Left,
    double Right,
    double Top,
    string Text);

/// <summary>
/// Reads the word boxes out of a PDF with <c>pdftotext</c>.
/// </summary>
/// <remarks>
/// <para>
/// The complement to <see cref="PdfTextRuns"/>, and the right tool for a different question. A run
/// comparison checks where the pen was when a portion started, which is what proves a line's placement;
/// a word comparison checks where every word inside a line sits, which is what proves justification —
/// where the portions are unchanged and only the blanks between the words are wider.
/// </para>
/// <para>
/// A box's left edge is the pen at the start of the word rather than the first glyph's ink: poppler
/// derives it from the text position and the font's advances. That is what makes it comparable with a
/// layout engine's own arithmetic to within a fraction of a point.
/// </para>
/// </remarks>
public static class PdfWords
{
    /// <summary>The words in a PDF, in the order <c>pdftotext</c> reports them.</summary>
    /// <remarks>
    /// Reading order per page, which for uniformly sized text is also logical order. It is not for a line
    /// that mixes sizes: poppler groups words by vertical position, so a 22 pt word on an 11 pt line is
    /// reported apart from its neighbours. Use <see cref="PdfTextRuns"/> for those.
    /// </remarks>
    /// <param name="pdfPath">The PDF to read.</param>
    /// <returns>The words, or an empty list when <c>pdftotext</c> is not installed.</returns>
    public static List<PdfWord> Read(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        ProcessStartInfo start = new("pdftotext")
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        start.ArgumentList.Add("-bbox");
        start.ArgumentList.Add(pdfPath);
        start.ArgumentList.Add("-");

        using Process process = Process.Start(start)
                                ?? throw new InvalidOperationException("pdftotext did not start");

        string output = process.StandardOutput.ReadToEnd();
        process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);

        return process.ExitCode == 0 ? Parse(output) : [];
    }

    /// <summary>True when <c>pdftotext</c> can be run at all.</summary>
    public static bool IsAvailable
    {
        get
        {
            try
            {
                using Process? probe = Process.Start(new ProcessStartInfo("pdftotext")
                {
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    ArgumentList = { "-v" },
                });

                probe?.WaitForExit(10000);
                return probe is not null;
            }
            catch (Exception exception) when (exception is System.ComponentModel.Win32Exception
                                                 or InvalidOperationException)
            {
                return false;
            }
        }
    }

    private static List<PdfWord> Parse(string output)
    {
        List<PdfWord> words = [];
        int page = 0;

        foreach (Match sheet in Regex.Matches(
                     output, "<page[^>]*>(.*?)</page>", RegexOptions.Singleline))
        {
            foreach (Match word in Regex.Matches(
                         sheet.Groups[1].Value,
                         "<word xMin=\"([0-9.]+)\" yMin=\"([0-9.]+)\" xMax=\"([0-9.]+)\""
                         + "[^>]*>([^<]*)</word>"))
            {
                words.Add(new PdfWord(
                    page,
                    Number(word.Groups[1].Value),
                    Number(word.Groups[3].Value),
                    Number(word.Groups[2].Value),
                    System.Net.WebUtility.HtmlDecode(word.Groups[4].Value)));
            }

            page++;
        }

        return words;
    }

    private static double Number(string value)
        => double.Parse(value, CultureInfo.InvariantCulture);
}
