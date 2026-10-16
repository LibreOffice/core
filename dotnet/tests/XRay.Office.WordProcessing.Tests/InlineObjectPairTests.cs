using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Two adjacent inline objects, too wide between them for one line, are split across two.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is a control, and it exists because a previous round concluded the opposite.</b> Round
/// 124 (<c>probes/ink-hunt-r124</c> §7) read the line-break tables and argued that no such split
/// can ever happen: <c>U+0001</c>, the anchor character every word-processing reader puts where an
/// inline object sits, is UAX #14 <c>Line_Break=CM</c>, and LB9 forbids a break <em>before</em> a
/// combining mark — so two adjacent anchors are welded and the second object can never leave the
/// first object's line. The reading of the tables is correct; the conclusion is not.
/// </para>
/// <para>
/// <c>inline-object-pair.docx</c> holds two inline pictures 4.5 in wide in one paragraph whose
/// measure is 6.5 in, so the pair overflows and the second must move. This tree draws them at
/// <c>y</c> 72.0–153.0 and 153.0–234.0, and 26.2.4.2 draws them at the same two rectangles. A
/// second probe at the witness's own geometry — 6.5 in and 6.5 in — agrees as well: ours 72.0–258.6
/// and 258.6–471.0 against 71.9–258.5 and 258.6–471.0. The anchors never reach LB9 as a pair: a run
/// holding nothing but anchors goes through <c>MeasuredParagraph</c>'s own path, where an inline
/// object widens every prefix past its boundary and the object is pushed to the next line when
/// anything precedes it.
/// </para>
/// <para>
/// So the defect round 124 saw on <c>RMI_…GettingOffOil.doc</c> — two as-char pictures drawn on one
/// baseline, one over the other — is not this mechanism. See <c>probes/wordsgroup-r128/results.md</c>
/// §3, which seated it against the WW8 reader.
/// </para>
/// <para>
/// <b>And that seat is closed, by the <c>.doc</c> half of this pair.</b> The WW8 walk dropped the
/// anchor character of every frame it made, floating or as-character, so two adjacent inline pictures
/// arrived at the layout with <em>one</em> offset between them — and an inline object's offset is a
/// boundary, so there was nothing for the measurer to break at. <c>inline-object-pair.doc</c> is
/// 26.2.4.2's own DOC export of two 4.4 in pictures in one paragraph on a 6.925 in measure; at the base
/// of round 130 this tree drew them at <c>y</c> 84.9–171.3 and 70.5–171.3, sharing a bottom edge, and it
/// now draws 70.5–156.9 and 156.9–257.7 against the reference's 70.6–157.0 and 157.0–257.6.
/// </para>
/// </remarks>
public sealed class InlineObjectPairTests
{
    /// <summary>Each object on its own line, and the second below the first.</summary>
    [Theory]
    [InlineData("inline-object-pair.docx")]
    [InlineData("inline-object-pair.doc")]
    public void TwoWideInlineObjectsTakeTwoLines(string name)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(name)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        IReadOnlyList<DocRect> images = [.. sink.Pages.SelectMany(page => page.Images)];
        images.Count.ShouldBe(2);

        // Not one on top of the other: the second starts at the first's bottom edge.
        images[1].Y.Points.ShouldBeGreaterThan(images[0].Y.Points + (images[0].Height.Points / 2));
        images[1].Y.Points.ShouldBe(
            images[0].Y.Points + images[0].Height.Points, 1.0);
    }
}
