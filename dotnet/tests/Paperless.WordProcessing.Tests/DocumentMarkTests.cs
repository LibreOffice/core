using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What the four readers now <em>record</em> over the text rather than resolving away: a tracked
/// change's author and words, a bookmark's range, and a field's definition beside its cached result.
/// </summary>
/// <remarks>
/// <para>
/// Two documents, each converted from one hand-written RTF into all four formats, so that agreement
/// between the readers is evidence about the readers rather than about a shared code path. The four
/// spell every one of these differently — a <c>w:ins</c> element against a <c>sprmCFRMarkIns</c>
/// against a <c>\revised</c> toggle against a hoisted <c>text:changed-region</c>; a
/// <c>w:bookmarkStart</c> paired by id against two PLCFs paired by an index one holds into the other
/// against a <c>{\*\bkmkstart}</c> paired by name against a <c>text:bookmark-start</c> — and they
/// still land on the same offsets in the same paragraphs.
/// </para>
/// <para>
/// <see cref="TrackedChangeTests"/> is the other half of this and must keep passing: recording a
/// change did not start emitting the text it removed.
/// </para>
/// </remarks>
public class DocumentMarkTests
{
    private const string Deleted = "a deleted phrase ";
    private const string Inserted = "an inserted phrase ";

    private static IWordProcessingDocument Open(string name)
        => (IWordProcessingDocument)new WordProcessingReader().Read(
            DocumentSource.FromFile(Corpus.Require(name)));

    [Theory]
    [InlineData("revisions.rtf")]
    [InlineData("revisions.doc")]
    [InlineData("revisions.docx")]
    [InlineData("revisions.odt")]
    public void AChangeIsRecordedWithItsAuthorAndItsWords(string name)
    {
        using IWordProcessingDocument document = Open(name);
        WritingMarks marks = document.Marks;

        marks.Changes.Count.ShouldBe(2, $"{name}: one insertion and one deletion");

        WritingChange insertion = marks.Changes.Single(c => c.Kind == WritingChangeKind.Insertion);
        insertion.Author.ShouldBe("Ada Lovelace");
        insertion.Text.ShouldBe(Inserted);

        // The insertion covers text that is in the document, so its range is the text's.
        insertion.Range.IsEmpty.ShouldBeFalse();
        insertion.Range.Start.Paragraph.Text
            .Substring(insertion.Range.Start.Offset, insertion.Range.End.Offset - insertion.Range.Start.Offset)
            .ShouldBe(Inserted);

        WritingChange deletion = marks.Changes.Single(c => c.Kind == WritingChangeKind.Deletion);
        deletion.Author.ShouldBe("Ada Lovelace");

        // The words the change removed, which the extraction deliberately does not have. This is the
        // only place they survive, and it is why WritingChange carries text at all.
        deletion.Text.ShouldBe(Deleted);

        // And an empty range, because there is nothing in the document between its two positions.
        deletion.Range.IsEmpty.ShouldBeTrue(
            $"{name}: a deletion covers no extracted text, so its range collapses onto the position "
            + "the text was removed from");

        // Both changes are in the paragraph that carries them, at the offsets the text says.
        string paragraph = deletion.Range.Start.Paragraph.Text;
        paragraph.ShouldBe("A paragraph with an inserted phrase and in the middle.");
        paragraph.IndexOf(Inserted, StringComparison.Ordinal).ShouldBe(insertion.Range.Start.Offset);
        deletion.Range.Start.Offset.ShouldBe(paragraph.IndexOf(" in the middle.", StringComparison.Ordinal) + 1);
    }

    /// <summary>
    /// The four formats record the same changes at the same offsets.
    /// </summary>
    /// <remarks>
    /// The timestamp is deliberately not compared, and the reason is a fact about LibreOffice rather
    /// than about the formats: the source RTF's <c>\revdttm</c> is zero, meaning "no date", and its
    /// own exporters disagree about what to do with that. The ODF and DOC exports both write the Unix
    /// epoch — <c>1970-01-01</c> and the <c>DTTM</c> that decodes to it — while the DOCX export omits
    /// <c>w:date</c> altogether and the RTF passes the zero through. So two of the four report a date
    /// and two report none, from one source document.
    /// </remarks>
    [Fact]
    public void TheFourFormatsRecordTheSameChanges()
    {
        (string Kind, string? Author, int Start, int End, string? Text)[] Read(string name)
        {
            using IWordProcessingDocument document = Open(name);
            return [.. document.Marks.Changes.Select(c =>
                (c.Kind.ToString(), c.Author, c.Range.Start.Offset, c.Range.End.Offset, c.Text))];
        }

        var expected = Read("revisions.rtf");
        Read("revisions.doc").ShouldBe(expected);
        Read("revisions.docx").ShouldBe(expected);
        Read("revisions.odt").ShouldBe(expected);
    }

    /// <summary>
    /// A date, where the file states one at all.
    /// </summary>
    /// <remarks>
    /// Pinned per format because the two that carry a date reach it by completely different routes: a
    /// packed 32-bit <c>DTTM</c> whose fields are not adjacent, and an ISO <c>dc:date</c> with no
    /// time. Agreeing on the same instant from those two is what says both are decoded right.
    /// </remarks>
    [Theory]
    [InlineData("revisions.doc", true)]
    [InlineData("revisions.odt", true)]
    [InlineData("revisions.docx", false)]
    [InlineData("revisions.rtf", false)]
    public void ADateIsRecordedWhereTheFileStatesOne(string name, bool hasDate)
    {
        using IWordProcessingDocument document = Open(name);

        foreach (WritingChange change in document.Marks.Changes)
        {
            if (hasDate) change.Timestamp.ShouldBe(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Unspecified));
            else change.Timestamp.ShouldBeNull();
        }
    }

    [Theory]
    [InlineData("bookmark-field.rtf")]
    [InlineData("bookmark-field.doc")]
    [InlineData("bookmark-field.docx")]
    [InlineData("bookmark-field.odt")]
    public void ABookmarkIsARange(string name)
    {
        using IWordProcessingDocument document = Open(name);
        WritingMarks marks = document.Marks;

        marks.Bookmarks.Count.ShouldBe(2, $"{name}: one spanning bookmark and one point");

        WritingBookmark span = marks.Bookmarks.Single(b => b.Name == "target");
        span.IsPoint.ShouldBeFalse();
        span.Range.Start.Paragraph.Text
            .Substring(span.Range.Start.Offset, span.Range.End.Offset - span.Range.Start.Offset)
            .ShouldBe("a marked phrase");

        // A collapsed bookmark is a legitimate one rather than a malformed one — ODF has a single
        // element for it — and it is the case that vanishes if WW8's two position tables are walked
        // as an ordered stream of starts and ends rather than looked up.
        WritingBookmark point = marks.Bookmarks.Single(b => b.Name == "here");
        point.IsPoint.ShouldBeTrue();
        point.Range.Start.Paragraph.Text.ShouldBe("A point bookmark in this paragraph.");
        point.Range.Start.Offset.ShouldBe("A point".Length);
    }

    /// <summary>
    /// A field keeps both halves: what it says to compute, and what was last computed.
    /// </summary>
    /// <remarks>
    /// The instruction is null for ODF and only for ODF, because ODF has none: a field there is a
    /// typed element and <see cref="WritingField.Kind"/> carries the whole of its meaning. That the
    /// kinds still agree across all four is the point of mapping both spellings onto one vocabulary.
    /// </remarks>
    [Theory]
    [InlineData("bookmark-field.rtf", "PAGE")]
    [InlineData("bookmark-field.doc", "PAGE")]
    [InlineData("bookmark-field.docx", "PAGE")]
    [InlineData("bookmark-field.odt", null)]
    public void AFieldKeepsItsDefinitionAndItsCachedResult(string name, string? pageInstruction)
    {
        using IWordProcessingDocument document = Open(name);
        WritingMarks marks = document.Marks;

        marks.Fields.Select(f => f.Kind).ShouldBe(
            [WritingFieldKind.PageNumber, WritingFieldKind.PageCount, WritingFieldKind.Reference]);

        WritingField page = marks.Fields[0];
        page.Instruction.ShouldBe(pageInstruction);

        // The cached result, which is what a reference renderer shows and what stays preferred by
        // default. LibreOffice's ODF export caches a page number of 0 where the other three cache 1
        // — the difference is in the file, not in the reading of it.
        page.Result.ShouldBe(name.EndsWith(".odt", StringComparison.Ordinal) ? "0" : "1");

        WritingField reference = marks.Fields[2];
        reference.Result.ShouldBe("a marked phrase");
        reference.Instruction.ShouldBe(
            name.EndsWith(".odt", StringComparison.Ordinal) ? null : @"REF target \h");

        // The result is where the document says it is, and the extracted text is unchanged by any of
        // this: the field still contributes its cached result and nothing else.
        document.Content.GetText().ShouldContain("Page " + page.Result + " of 1.");
    }

    /// <summary>
    /// A document with none of the three records nothing, which is what keeps extraction free.
    /// </summary>
    /// <remarks>
    /// The whole design rests on this: paragraphs are materialised only where a mark needs a position,
    /// so an unmarked document allocates one integer per paragraph and no model at all.
    /// </remarks>
    [Theory]
    [InlineData("mixed-runs.odt")]
    [InlineData("mixed-runs.docx")]
    [InlineData("mixed-runs.rtf")]
    [InlineData("mixed-runs.doc")]
    public void ADocumentWithNoMarksRecordsNothing(string name)
    {
        using IWordProcessingDocument document = Open(name);
        document.Marks.IsEmpty.ShouldBeTrue();
    }
}
