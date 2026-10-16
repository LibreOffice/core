namespace XRay.Office.WordProcessing;

/// <summary>
/// Which application this reader agrees with where LibreOffice and Word read the same file
/// differently: <b>Word, by default</b>.
/// </summary>
/// <remarks>
/// <para>
/// Nearly everything this project measures is a place where LibreOffice is right about a file and
/// this tree was wrong, and those are simply fixed. A handful are the other way round — the
/// reference does something to the document that the application that wrote it does not — and they
/// need a decision rather than a fix, because the gate scores this tree against 26.2.4.2's own
/// output and reproducing the quirk is what makes the gate green.
/// </para>
/// <para>
/// <strong>The decision is Word.</strong> A reader wants the document as its author saw it, so
/// where the two disagree this tree follows the writing application and accepts the gate row.
/// Every such place is implemented anyway, behind <see cref="Variable"/>, so a run that wants to
/// agree with a local LibreOffice can have it — and so that the measurement survives, which is the
/// expensive half. <c>dotnet/TODO.word-parity.md</c> lists them, what each one costs, and the
/// corpus rows to stop chasing.
/// </para>
/// <para>
/// This is deliberately one switch rather than one per rule. It is a statement about which
/// application the output should resemble, and a caller that wants half of LibreOffice's answer
/// and half of Word's wants neither.
/// </para>
/// </remarks>
internal static class WordParity
{
    /// <summary>The variable that asks for LibreOffice's reading instead of Word's.</summary>
    /// <remarks>
    /// <c>1</c>, <c>true</c> or <c>yes</c> reproduces 26.2.4.2; anything else, including unset,
    /// keeps Word's answer. Read that way round so a variable set to an unexpected value leaves
    /// the default in place — the default is what a reader wants and the alternative is a quirk.
    /// </remarks>
    public const string Variable = "XRAY_OFFICE_LIBREOFFICE_QUIRKS";

    /// <summary>True while the reference's readings are being reproduced rather than Word's.</summary>
    public static bool ReproduceLibreOffice =>
        Environment.GetEnvironmentVariable(Variable) is "1" or "true" or "yes";
}
