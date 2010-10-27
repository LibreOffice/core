/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// MsOfficeDocumentInformation.h: Schnittstelle f�r die Klasse MsOfficeDocumentInformation.
//
//////////////////////////////////////////////////////////////////////

#ifndef _REGISTRATIONCONTEXTINFORMATION_HXX_
#define _REGISTRATIONCONTEXTINFORMATION_HXX_

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string>

/** A simple implementation class that returns the
    appropriate display names for the Microsoft
    Office document types.
    Under Windows 9x this class checks if the
    document display name is convertable to an ANSI
    string and if not returns an english default.
    So we avoid garbage if soemone for instance
    installs an japanese StarOffice/OpenOffice
    under a German Windows 98 for instance.
*/
class RegistrationContextInformation
{
public:

    enum SHELL_COMMAND {New, Open, Print, Printto};
    enum OFFICE_APPLICATION {Office, Writer, Calc, Impress};

    RegistrationContextInformation(MSIHANDLE hMsi, const std::wstring& OpenOfficeExecutablePath);

    /** Word document information
        The icon index is the index of the icon
        in soffice.exe to be associated with
        word document files
    */
    std::wstring GetWordDocumentDisplayName() const;
    std::wstring GetWordDocumentFileExtension() const;
    std::wstring GetWordDocumentDefaultIconEntry() const;
    std::wstring GetWordDocumentDefaultShellCommand() const;

    /** Word template information
        The icon index is the index of the icon
        in soffice.exe to be associated with
        word template files
    */
    std::wstring GetWordTemplateDisplayName() const;
    std::wstring GetWordTemplateFileExtension() const;
    std::wstring GetWordTemplateDefaultIconEntry() const;
    std::wstring GetWordTemplateDefaultShellCommand() const;

    /** Rtf document information
        The icon index is the index of the icon
        in soffice.exe to be associated with
        rtf document files
    */
    std::wstring GetRtfDocumentDisplayName() const;
    std::wstring GetRtfDocumentFileExtension() const;
    std::wstring GetRtfDocumentDefaultIconEntry() const;
    std::wstring GetRtfDocumentDefaultShellCommand() const;

    /** Excel sheet information
        The icon index is the index of the icon
        in soffice.exe to be associated with
        Excel sheets
    */
    std::wstring GetExcelSheetDisplayName() const;
    std::wstring GetExcelSheetFileExtension() const;
    std::wstring GetExcelSheetDefaultIconEntry() const;
    std::wstring GetExcelSheetDefaultShellCommand() const;

    /** Excel template information
        The icon index is the index of the icon
        in soffice.exe to be associated with
        Excel template files
    */
    std::wstring GetExcelTemplateDisplayName() const;
    std::wstring GetExcelTemplateFileExtension() const;
    std::wstring GetExcelTemplateDefaultIconEntry() const;
    std::wstring GetExcelTemplateDefaultShellCommand() const;

    /** PowerPoint document information
        The icon index is the index of the icon
        in soffice.exe to be associated with
        PowerPoint document files
    */
    std::wstring GetPowerPointDocumentDisplayName() const;
    std::wstring GetPowerPointDocumentFileExtension() const;
    std::wstring GetPowerPointDocumentDefaultIconEntry() const;
    std::wstring GetPowerPointDocumentDefaultShellCommand() const;

    /** PowerPoint template information
        The icon index is the index of the icon
        in soffice.exe to be associated with
        PowerPoint template files
    */
    std::wstring GetPowerPointTemplateDisplayName() const;
    std::wstring GetPowerPointTemplateFileExtension() const;
    std::wstring GetPowerPointTemplateDefaultIconEntry() const;
    std::wstring GetPowerPointTemplateDefaultShellCommand() const;

    /** PowerPoint Show information
    */
    std::wstring GetPowerPointShowDisplayName() const;
    std::wstring GetPowerPointShowFileExtension() const;
    std::wstring GetPowerPointShowDefaultIconEntry() const;
    std::wstring GetPowerPointShowDefaultShellCommand() const;

    /** The string for the "New" command that should appear
        in the Explorer context menu when someone right
        clicks a Microsoft document
    */
    std::wstring ShellNewCommandDisplayName() const;

    /** The string for the "Edit" command that should
        appear in the Explorer context menu when someone
        right clicks a document
    */
    std::wstring ShellEditCommandDisplayName() const;

    /** A friendly name for the application
    */
    std::wstring GetOpenOfficeFriendlyAppName() const;

    /** The path to the StarOffice/OpenOffice executable
    */
    std::wstring GetOpenOfficeExecutablePath() const;

    /** The name of the executable (currently "soffice.exe"
        but may change in the future, who knows)
    */
    std::wstring GetOpenOfficeExecutableName() const;

    /** A command line for the specified shell command
    */
    std::wstring GetOpenOfficeCommandline(SHELL_COMMAND ShellCommand,
                                          OFFICE_APPLICATION OfficeApp) const;

private:
    bool IsConvertableToAnsi(const std::wstring& String)  const;

    void ExtractOpenOfficeExecNameFromPath();

private:
    MSIHANDLE    msihandle_;
    bool         m_IsWin9x;
    std::wstring m_OOExecPath;
    std::wstring m_OOExecName;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
