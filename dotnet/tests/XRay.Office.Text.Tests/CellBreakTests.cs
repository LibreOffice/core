using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A stretch that breaks between characters rather than between words.
/// </summary>
/// <remarks>
/// <para>
/// EditEngine's field. A field is one portion of the line whatever its length, and when it is
/// wider than the room left it is neither moved down nor offered to the break iterator:
/// <c>ImpEditEngine::CreateLines</c>' <c>EE_FEATURE_FIELD</c> branch walks the field's own text
/// with <c>XBreakIterator::nextCharacters(…, CharacterIteratorMode::SKIPCELL, …)</c> and records a
/// break wherever the next cell would overflow
/// (<c>editeng/source/editeng/impedit3.cxx</c>:1101-1200). A cell is a grapheme cluster, so on
/// Latin text that is an opportunity at every character.
/// </para>
/// <para>
/// Two behaviours, and the second is as important as the first. The opportunities are
/// <em>added</em> to the break iterator's rather than replacing them, so nothing about a stretch
/// that fits changes. And the solidus glue — see <see cref="SolidusGlueTests"/> — must not fire on
/// a break inside one, because that rule belongs to
/// <c>BreakIterator_Unicode::getLineBreak</c> and a field asks it nothing.
/// </para>
/// </remarks>
public class CellBreakTests
{
    private static readonly Length Size = Length.FromPoints(10);

    /// <summary>The URL a slide's hyperlink is, without the blanks a word breaker needs.</summary>
    private const string Url = "https://example.org/one-two/three-four/five";

    /// <summary>
    /// Without the stretch the line breaks at a separator; with it, at the fitting limit.
    /// </summary>
    [Fact]
    public void ACellBrokenStretchBreaksWhereTheWidthRunsOutRatherThanAtASeparator()
    {
        // A width that ends part way through `two`, so the last separator that fits is the
        // hyphen four characters earlier.
        (LineFiller filler, MeasuredParagraph measured, Length width) = Fill(Url, Url[..26]);

        int plain = filler.Fill(measured, width)[0].End;
        int cell = filler.Fill(measured, width, cellBroken: [new CellBrokenSpan(0, Url.Length)])[0].End;

        // The assertion is what the two breaks *are*, not where: UAX #14 can only end a line
        // after one of this string's separators, and the cell rule ends it wherever the width
        // ran out — which is what a URL breaking mid-token looks like.
        Url[plain - 1].ShouldBeOneOf('/', '-');
        Url[cell - 1].ShouldNotBeOneOf('/', '-');
        cell.ShouldBeGreaterThan(plain);
    }

    /// <summary>
    /// The glue that pulls a break back from behind a solidus does not apply inside a field.
    /// </summary>
    /// <remarks>
    /// The pair is the whole rule: the same text, the same width, and the only difference is
    /// whether the URL is a field. Without the suppression the break after the solidus is pulled
    /// back to the blank behind <c>See</c> and the whole path moves onto the next line, which is
    /// what LibreOffice does for text and not for a field.
    /// </remarks>
    [Fact]
    public void TheSolidusGlueDoesNotFireInsideACellBrokenStretch()
    {
        const string text = "See https://a.example/bb/";
        (LineFiller filler, MeasuredParagraph measured, Length width) = Fill(text, text);

        // Narrow enough that the trailing solidus is the last break that fits.
        Length narrow = width - Length.FromPoints(1);

        filler.Fill(measured, narrow)[0].End.ShouldBe(4);
        filler.Fill(measured, narrow, cellBroken: [new CellBrokenSpan(4, text.Length - 4)])[0]
            .End.ShouldBeGreaterThan(4);
    }

    /// <summary>A stretch that fits its line is not broken at all.</summary>
    /// <remarks>
    /// The control for the whole feature. A field's rule only ever fires when the field overflows,
    /// so a document whose links are short must lay out identically with and without it.
    /// </remarks>
    [Fact]
    public void AStretchThatFitsIsNotBroken()
    {
        (LineFiller filler, MeasuredParagraph measured, Length width) = Fill(Url, Url);

        filler.Fill(measured, width, cellBroken: [new CellBrokenSpan(0, Url.Length)])
            .Count.ShouldBe(1);
    }

    /// <summary>A break at the stretch's own start or end is not a break <em>inside</em> it.</summary>
    /// <remarks>
    /// Strictly inside, which is what the two consumers of this predicate want: a line beginning at
    /// the stretch's start is the line the field <em>starts</em> on and not one it spilled onto, so
    /// it takes an ordinary line height, and the solidus glue at an edge belongs to the text around
    /// the stretch. It is <em>not</em> a statement about which opportunities exist — see
    /// <see cref="TheStretchesOwnStartIsAnOpportunity"/>.
    /// </remarks>
    [Fact]
    public void TheStretchesOwnEdgesAreNotBreaksInsideIt()
    {
        new CellBrokenSpan(4, 6).BreaksInside(4).ShouldBeFalse();
        new CellBrokenSpan(4, 6).BreaksInside(5).ShouldBeTrue();
        new CellBrokenSpan(4, 6).BreaksInside(10).ShouldBeFalse();
    }

    /// <summary>
    /// The stretch's own start is an opportunity, so a field whose first cell does not fit moves
    /// down whole even where the language offers no break in front of it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lineBreaksList.push_back(0)</c> is unconditional in the branch — <em>"always add 1st line
    /// break"</em> — and it is what <c>bFieldStartNextLine</c> uses
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:1148-1149, :1173-1180).
    /// </para>
    /// <para>
    /// The text here is the shape a corpus deck actually writes: a link inside brackets, where the
    /// opening bracket is <c>OP</c> and so prohibits a break after itself. 26.2.4.2 leaves the
    /// <c>(</c> at the end of the line and puts the whole link on the next one; without this the
    /// line breaks in front of the bracket. Measured on <c>probes/pptx-field-r82/</c>'s <c>g23</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheStretchesOwnStartIsAnOpportunity()
    {
        const string text = "aaa bbb (https://example.org/x";
        int link = text.IndexOf("https", StringComparison.Ordinal);

        (LineFiller filler, MeasuredParagraph measured, Length width) = Fill(text, text[..link]);

        // Room for everything up to and including the bracket, and not for the link's first
        // character. Without the stretch the only opportunity there is in front of the bracket.
        filler.Fill(measured, width)[0].End.ShouldBe(8);
        filler.Fill(measured, width, cellBroken: [new CellBrokenSpan(link, text.Length - link)])[0]
            .End.ShouldBe(link);
    }

    /// <summary>
    /// A filler over Carlito, the paragraph measured across one run, and the width of a prefix.
    /// </summary>
    private static (LineFiller Filler, MeasuredParagraph Measured, Length Width) Fill(
        string text, string upTo)
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
        TextMeasurer measurer = new(face);

        return (
            new LineFiller(measurer),
            MeasuredParagraph.Measure(text, [new FormattedRun(0, text.Length, face, Size)]),
            measurer.Measure(upTo, Size));
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string? found = Directory
                .EnumerateFiles(directory, fileName, SearchOption.AllDirectories)
                .FirstOrDefault();
            if (found is not null) return found;
        }

        return null;
    }
}
