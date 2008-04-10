// Registrar.cpp: Implementierung der Klasse Registrar.
//
//////////////////////////////////////////////////////////////////////

#include "registrar.hxx"

#ifndef _REGISTRYVALUEIMPL_HXX_
#include "RegistryValueImpl.hxx"
#endif
#include "windowsregistry.hxx"
#include "registryexception.hxx"

#include <assert.h>
#ifdef _MSC_VER
#pragma warning(disable: 4350 4482)
#include "strsafe.h"
#endif

//----------------------------------------------------------
#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    TCHAR    buffer[1024];
    va_list  args;

    va_start( args, pFormat );
    StringCchVPrintf( buffer, sizeof(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif
//----------------------------------------------------------

const int MSWORD                     = 0x1;
const int MSEXCEL                    = 0x2;
const int MSPOWERPOINT               = 0x4;
const int DEFAULT_HTML_EDITOR_FOR_IE = 0x8;
const int HTML_EDITOR                = 0x10;
const int DEFAULT_SHELL_HTML_EDITOR  = 0x20;

namespace /* private */
{
    const std::wstring HTM_OPENWITHLIST = L".htm\\OpenWithList";
    const std::wstring APPLICATIONS = L"Applications";
    const std::wstring SHELL_EDIT_COMMAND = L"shell\\edit\\command";
    const std::wstring HTML_EDIT = L"HTML Edit";
    const std::wstring HTML_EDIT_DISPLAY_NAME = L"Edit Display Name";
    const std::wstring SHELL_EDIT_COMMAND_BACKUP = L"Shell Edit Cmd";
    const std::wstring DEFAULT_HTML_EDITOR = L"Default HTML Editor";
    const std::wstring MS_IE_DEF_HTML_EDITOR = L"Software\\Microsoft\\Internet Explorer\\Default HTML Editor";
    const std::wstring MS_IE_DEF_HTML_EDITOR_SHL_EDIT_CMD = L"Software\\Microsoft\\Internet Explorer\\Default HTML Editor\\shell\\edit\\command";
}

Registrar::Registrar(const RegistrationContextInformation& RegContext) :
    m_ContextInformation(RegContext),
    FORWARD_KEY_PREFIX(L"OpenOffice.org"),//FORWARD_KEY_PREFIX(L"soffice6"),
    DEFAULT_VALUE_NAME(L""),
    BACKUP_VALUE_NAME(L"Backup"),
    PRIVATE_BACKUP_KEY_NAME(L"OpenOffice.org.reg4msdocmsi"),//PRIVATE_BACKUP_KEY_NAME(L"soffice6.bak"),
    REGISTRATION_STATE(L"Reg4MsDocState")
{
    m_RootKey = WindowsRegistry().GetClassesRootKey();
}

Registrar::~Registrar()
{
}

void Registrar::RegisterForMsWord() const
{
    assert(m_RootKey.get());

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetWordDocumentFileExtension(),
        m_ContextInformation.GetWordDocumentDisplayName(),
        m_ContextInformation.GetWordDocumentDefaultIconEntry(),
        m_ContextInformation.GetWordDocumentDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Writer);

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetWordTemplateFileExtension(),
        m_ContextInformation.GetWordTemplateDisplayName(),
        m_ContextInformation.GetWordTemplateDefaultIconEntry(),
        m_ContextInformation.GetWordTemplateDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Writer);

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetRtfDocumentFileExtension(),
        m_ContextInformation.GetRtfDocumentDisplayName(),
        m_ContextInformation.GetRtfDocumentDefaultIconEntry(),
        m_ContextInformation.GetRtfDocumentDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Writer);

    SaveRegisteredFor(MSWORD);
}

void Registrar::UnregisterForMsWord() const
{
    assert(m_RootKey.get());

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetWordDocumentFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetWordTemplateFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetRtfDocumentFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    SaveNotRegisteredFor(MSWORD);
}

bool Registrar::QueryPreselectForMsApplication(const std::wstring& file_extension) const
{
    bool preselect = false;

    // We use HKCR else we would not see that a registration for
    // MS Office applications already exist if we are about to
    // register in HKCU\Software\Classes
    RegistryKey root_key = WindowsRegistry().GetClassesRootKey();

    if (!root_key->HasSubKey(file_extension))
    {
        preselect = true;
        OutputDebugStringFormat( TEXT("QueryPreselect: No SubKey found for (%s), preselected!\n"), file_extension.c_str() );
    }
    else
    {
        RegistryKey RegKey = root_key->OpenSubKey(file_extension, false);

        if (RegKey->HasValue(DEFAULT_VALUE_NAME))
        {
            RegistryValue RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

            if (REG_SZ == RegVal->GetType() &&
                IsOpenOfficeRegisteredForMsApplication(RegVal->GetDataAsUniString()))
            {
                preselect = true;
                OutputDebugStringFormat( TEXT("QueryPreselect: (%s) registered to Office, preselected!\n"), file_extension.c_str() );
            }
            else if ( (REG_SZ == RegVal->GetType()) && ! root_key->HasSubKey( RegVal->GetDataAsUniString() ) )
            {
                preselect = true;
                OutputDebugStringFormat( TEXT("QueryPreselect: (%s) registered but destination is empty, preselected!\n"), file_extension.c_str() );
            }
        }
        else
        {
            preselect = true;
            OutputDebugStringFormat( TEXT("QueryPreselect: No default found for SubKey (%s), preselected!\n"), file_extension.c_str() );
        }
    }
    return preselect;
}

bool Registrar::QueryPreselectMsWordRegistration() const
{
    return QueryPreselectForMsApplication(
        m_ContextInformation.GetWordDocumentFileExtension());
}

void Registrar::RegisterForMsExcel() const
{
    assert(m_RootKey.get());

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetExcelSheetFileExtension(),
        m_ContextInformation.GetExcelSheetDisplayName(),
        m_ContextInformation.GetExcelSheetDefaultIconEntry(),
        m_ContextInformation.GetExcelSheetDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Calc);

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetExcelTemplateFileExtension(),
        m_ContextInformation.GetExcelTemplateDisplayName(),
        m_ContextInformation.GetExcelTemplateDefaultIconEntry(),
        m_ContextInformation.GetExcelTemplateDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Calc);

    SaveRegisteredFor(MSEXCEL);
}

void Registrar::UnregisterForMsExcel() const
{
    assert(m_RootKey.get());

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetExcelSheetFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetExcelTemplateFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    SaveNotRegisteredFor(MSEXCEL);
}

bool Registrar::QueryPreselectMsExcelRegistration() const
{
    return QueryPreselectForMsApplication(
        m_ContextInformation.GetExcelSheetFileExtension());
}

void Registrar::RegisterForMsPowerPoint() const
{
    assert(m_RootKey.get());

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetPowerPointDocumentFileExtension(),
        m_ContextInformation.GetPowerPointDocumentDisplayName(),
        m_ContextInformation.GetPowerPointDocumentDefaultIconEntry(),
        m_ContextInformation.GetPowerPointDocumentDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Impress);

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetPowerPointShowFileExtension(),
        m_ContextInformation.GetPowerPointShowDisplayName(),
        m_ContextInformation.GetPowerPointShowDefaultIconEntry(),
        m_ContextInformation.GetPowerPointShowDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Impress);

    RegisterForMsOfficeApplication(
        m_ContextInformation.GetPowerPointTemplateFileExtension(),
        m_ContextInformation.GetPowerPointTemplateDisplayName(),
        m_ContextInformation.GetPowerPointTemplateDefaultIconEntry(),
        m_ContextInformation.GetPowerPointTemplateDefaultShellCommand(),
        m_ContextInformation.ShellNewCommandDisplayName(),
        RegistrationContextInformation::Impress);

    SaveRegisteredFor(MSPOWERPOINT);
}

void Registrar::UnregisterForMsPowerPoint() const
{
    assert(m_RootKey.get());

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetPowerPointDocumentFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetPowerPointShowFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    try
    {
        UnregisterForMsOfficeApplication(
            m_ContextInformation.GetPowerPointTemplateFileExtension());
    }
    catch(RegistryKeyNotFoundException&)
    {}

    SaveNotRegisteredFor(MSPOWERPOINT);
}

//-----------------------------------------
/*
*/
bool Registrar::QueryPreselectMsPowerPointRegistration() const
{
    return QueryPreselectForMsApplication( m_ContextInformation.GetPowerPointDocumentFileExtension()) &&
           QueryPreselectForMsApplication( m_ContextInformation.GetPowerPointShowFileExtension());
}

//-----------------------------------------
/** The documentation says we have to
      make the following entries to register
      a html editor for the Internet Explorer
      HKCR\.htm\OpenWithList\App Friendly Name\shell\edit\command
      But the reality shows that this works only
      with Internet Explorer 5.x
      Internet Explorer 6.0 wants the follwoing
      entries:
      HKCR\.htm\OpenWithList\App.exe
      HKCR\Applications\App.ex\shell\edit\command
*/
void Registrar::RegisterAsHtmlEditorForInternetExplorer() const
{
    assert(m_RootKey.get());

    std::wstring OOFriendlyAppName = m_ContextInformation.GetOpenOfficeFriendlyAppName();

    std::wstring RegKeyName = HTM_OPENWITHLIST + std::wstring(L"\\") + OOFriendlyAppName;
    RegistryKey RegKey = m_RootKey->CreateSubKey(RegKeyName);

    RegKey = RegKey->CreateSubKey(SHELL_EDIT_COMMAND);

    RegistryValue RegVal(
        new RegistryValueImpl(
            DEFAULT_VALUE_NAME,
            m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::Open,
                                                          RegistrationContextInformation::Writer)));

    RegKey->SetValue(RegVal);

    RegKeyName = APPLICATIONS + std::wstring(L"\\") + OOFriendlyAppName;
    RegKey = m_RootKey->CreateSubKey(RegKeyName);

    RegVal->SetName(L"FriendlyAppName");
    RegVal->SetValue(OOFriendlyAppName);
    RegKey->SetValue(RegVal);

    RegKey = RegKey->CreateSubKey(SHELL_EDIT_COMMAND);
    RegVal->SetName(DEFAULT_VALUE_NAME);
    RegVal->SetValue(
        m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::Open,
                                                      RegistrationContextInformation::Writer));
    RegKey->SetValue(RegVal);

    SaveRegisteredFor(HTML_EDITOR);
}

void Registrar::UnregisterAsHtmlEditorForInternetExplorer() const
{
    assert(m_RootKey.get());

    try
    {
        std::wstring OOFriendlyAppName = m_ContextInformation.GetOpenOfficeFriendlyAppName();

        RegistryKey aRegKey = m_RootKey->OpenSubKey( APPLICATIONS );
        if ( aRegKey->HasSubKey( OOFriendlyAppName ) )
            aRegKey->DeleteSubKeyTree( OOFriendlyAppName );

        aRegKey = m_RootKey->OpenSubKey( HTM_OPENWITHLIST );
        if ( aRegKey->HasSubKey( OOFriendlyAppName ) )
            aRegKey->DeleteSubKeyTree( OOFriendlyAppName );
    }
    catch(RegistryKeyNotFoundException&)
    {}

    SaveNotRegisteredFor(HTML_EDITOR);
}

void Registrar::RegisterAsDefaultHtmlEditorForInternetExplorer() const
{
    assert(m_RootKey.get());

    RegistryKey RegistrationRootKey = GetRootKeyForDefHtmlEditorForIERegistration();

    RegistryKey RegKey = RegistrationRootKey->CreateSubKey(MS_IE_DEF_HTML_EDITOR_SHL_EDIT_CMD);

    RegistryValue RegVal = RegistryValue(new RegistryValueImpl(DEFAULT_VALUE_NAME, L""));

    if (RegKey->HasValue(DEFAULT_VALUE_NAME))
    {
        RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

        std::wstring CmdLine = RegVal->GetDataAsUniString();

        if (std::wstring::npos == CmdLine.find(m_ContextInformation.GetOpenOfficeExecutableName()))
        {
            RegistryKey BackupRegKey = m_RootKey->CreateSubKey(PRIVATE_BACKUP_KEY_NAME + L"\\" + DEFAULT_HTML_EDITOR);

            if (RegKey->HasValue(DEFAULT_VALUE_NAME))
                BackupRegKey->CopyValue(RegKey, DEFAULT_VALUE_NAME);

            RegKey = RegistrationRootKey->OpenSubKey(MS_IE_DEF_HTML_EDITOR);
            if (RegKey->HasValue(L"Description"))
                BackupRegKey->CopyValue(RegKey, L"Description");
        }
    }

    RegVal->SetValue(
        m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::Open,
                                                      RegistrationContextInformation::Writer));
    RegKey = RegistrationRootKey->OpenSubKey(MS_IE_DEF_HTML_EDITOR_SHL_EDIT_CMD);
    RegKey->SetValue(RegVal);

    RegVal->SetName(L"Description");
    RegVal->SetValue(m_ContextInformation.GetOpenOfficeFriendlyAppName());
    RegKey = RegistrationRootKey->OpenSubKey(MS_IE_DEF_HTML_EDITOR);
    RegKey->SetValue(RegVal);

    SaveRegisteredFor(DEFAULT_HTML_EDITOR_FOR_IE);
}

void Registrar::UnregisterAsDefaultHtmlEditorForInternetExplorer() const
{
    assert(m_RootKey.get());

    RegistryKey RegistrationRootKey = GetRootKeyForDefHtmlEditorForIERegistration();

    RegistryKey RegKey = RegistrationRootKey->OpenSubKey(MS_IE_DEF_HTML_EDITOR_SHL_EDIT_CMD);

    if (RegKey->HasValue(DEFAULT_VALUE_NAME))
    {
        RegistryValue RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

        std::wstring CmdLine = RegVal->GetDataAsUniString();

        if (std::wstring::npos != CmdLine.find(m_ContextInformation.GetOpenOfficeExecutableName()))
        {
            RegistryKey BackupRegKey = m_RootKey->OpenSubKey(PRIVATE_BACKUP_KEY_NAME);

            if (BackupRegKey->HasSubKey(DEFAULT_HTML_EDITOR))
            {
                BackupRegKey = BackupRegKey->OpenSubKey(DEFAULT_HTML_EDITOR);

                if (BackupRegKey->HasValue(DEFAULT_VALUE_NAME))
                    RegKey->CopyValue(BackupRegKey, DEFAULT_VALUE_NAME);
                else
                    RegKey->DeleteValue(DEFAULT_VALUE_NAME);

                RegKey = RegistrationRootKey->OpenSubKey(MS_IE_DEF_HTML_EDITOR);

                if (BackupRegKey->HasValue(L"Description"))
                    RegKey->CopyValue(BackupRegKey, L"Description");
                else
                    RegKey->DeleteValue(L"Description");
            }
            else
            {
                RegKey->DeleteValue(DEFAULT_VALUE_NAME);
                RegKey = RegistrationRootKey->OpenSubKey(MS_IE_DEF_HTML_EDITOR);
                RegKey->DeleteValue(L"Description");
            }
        }
    }

    SaveNotRegisteredFor(DEFAULT_HTML_EDITOR_FOR_IE);
}

void Registrar::RegisterAsDefaultShellHtmlEditor() const
{
    assert(m_RootKey.get());

    RegistryKey RegKey = m_RootKey->CreateSubKey(L".htm");

    RegistryValue RegVal = RegistryValue(
        new RegistryValueImpl(DEFAULT_VALUE_NAME, L""));

    if (RegKey->HasValue(DEFAULT_VALUE_NAME))
        RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

    std::wstring HtmFwdKey = RegVal->GetDataAsUniString();
    if (0 == HtmFwdKey.length() || !m_RootKey->HasSubKey(HtmFwdKey))
        HtmFwdKey = L".htm";

    RegKey = m_RootKey->CreateSubKey(HtmFwdKey + L"\\" + SHELL_EDIT_COMMAND);

    if (RegKey->HasValue(DEFAULT_VALUE_NAME))
    {
        RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

        std::wstring CmdLine = RegVal->GetDataAsUniString();

        // backup old values if we are not in place
        if (std::wstring::npos == CmdLine.find(m_ContextInformation.GetOpenOfficeExecutableName()))
        {
            RegistryKey BackupRegKey = m_RootKey->CreateSubKey(PRIVATE_BACKUP_KEY_NAME + L"\\" + HTML_EDIT);
            BackupRegKey->CopyValue(RegKey, DEFAULT_VALUE_NAME, SHELL_EDIT_COMMAND_BACKUP);
        }
    }

    RegVal->SetValue(
        m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::Open,
                                                      RegistrationContextInformation::Writer));

    RegKey->SetValue(RegVal);

    SaveRegisteredFor(DEFAULT_SHELL_HTML_EDITOR);
}

void Registrar::UnregisterAsDefaultShellHtmlEditor() const
{
    assert(m_RootKey.get());

    try
    {
        RegistryKey RegKey = m_RootKey->OpenSubKey(L".htm");

        RegistryValue RegVal = RegistryValue(
            new RegistryValueImpl(DEFAULT_VALUE_NAME, L""));

        if (RegKey->HasValue(DEFAULT_VALUE_NAME))
            RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

        std::wstring HtmFwdKey = RegVal->GetDataAsUniString();

        if (0 == HtmFwdKey.length() || !m_RootKey->HasSubKey(HtmFwdKey))
            HtmFwdKey = L".htm";

        RegKey = m_RootKey->OpenSubKey(HtmFwdKey + L"\\" + SHELL_EDIT_COMMAND);

        RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

        std::wstring CmdLine = RegVal->GetDataAsUniString();

        if (std::wstring::npos != CmdLine.find(m_ContextInformation.GetOpenOfficeExecutableName()))
        {
            RegistryKey BackupRegKey = m_RootKey->CreateSubKey(PRIVATE_BACKUP_KEY_NAME + L"\\" + HTML_EDIT);

            if (BackupRegKey->HasValue(SHELL_EDIT_COMMAND_BACKUP))
                RegKey->CopyValue(BackupRegKey, SHELL_EDIT_COMMAND_BACKUP, DEFAULT_VALUE_NAME);
            else
                RegKey->DeleteValue(DEFAULT_VALUE_NAME);
        }
    }
    catch(RegistryKeyNotFoundException&)
    {
    }

    SaveNotRegisteredFor(DEFAULT_SHELL_HTML_EDITOR);
}

void Registrar::SaveRegisteredFor(int State) const
{
    assert(m_RootKey.get());

    int NewState = GetRegisterState();
    NewState |= State;
    SetRegisterState(NewState);
}

void Registrar::SaveNotRegisteredFor(int State) const
{
    assert(m_RootKey.get());

    int NewState = GetRegisterState();
    NewState &= ~State;
    SetRegisterState(NewState);
}

int Registrar::GetRegisterState() const
{
    int State = 0;

    RegistryKey RegKey = m_RootKey->CreateSubKey(PRIVATE_BACKUP_KEY_NAME);

    if (RegKey->HasValue(REGISTRATION_STATE))
    {
        RegistryValue RegVal = RegKey->GetValue(REGISTRATION_STATE);
        if (REG_DWORD == RegVal->GetType())
            State = RegVal->GetDataAsInt();
    }

    return State;
}

void Registrar::SetRegisterState(int NewState) const
{
    RegistryKey RegKey = m_RootKey->CreateSubKey(PRIVATE_BACKUP_KEY_NAME);
    RegistryValue RegVal = RegistryValue(new RegistryValueImpl(REGISTRATION_STATE, NewState));
    RegKey->SetValue(RegVal);
}

bool Registrar::IsRegisteredFor(int State) const
{
    assert(m_RootKey.get());

    RegistryKey RegKey = m_RootKey->CreateSubKey(PRIVATE_BACKUP_KEY_NAME);

    int SavedState = 0;

    if (RegKey->HasValue(REGISTRATION_STATE))
    {
        RegistryValue RegVal = RegKey->GetValue(REGISTRATION_STATE);
        if (REG_DWORD == RegVal->GetType())
            SavedState = RegVal->GetDataAsInt();
    }

    return ((SavedState & State) == State);
}

//--------------------------------------
/** Restore the last registration state (necessary for
    Setup repair) */
void Registrar::RepairRegistrationState() const
{
    assert(m_RootKey.get());

    if (IsRegisteredFor(MSWORD))
        RegisterForMsWord();

    if (IsRegisteredFor(MSEXCEL))
        RegisterForMsExcel();

    if (IsRegisteredFor(MSPOWERPOINT))
        RegisterForMsPowerPoint();

    if (IsRegisteredFor(DEFAULT_HTML_EDITOR_FOR_IE))
        RegisterAsDefaultHtmlEditorForInternetExplorer();

    if (IsRegisteredFor(HTML_EDITOR))
        RegisterAsHtmlEditorForInternetExplorer();

    if (IsRegisteredFor(DEFAULT_SHELL_HTML_EDITOR))
        RegisterAsDefaultShellHtmlEditor();
}

/** Unregisters all and delete all Registry keys we have written */
void Registrar::UnregisterAllAndCleanUpRegistry() const
{
    assert(m_RootKey.get());

    if (IsRegisteredFor(MSWORD))
        UnregisterForMsWord();

    if (IsRegisteredFor(MSEXCEL))
        UnregisterForMsExcel();

    if (IsRegisteredFor(MSPOWERPOINT))
        UnregisterForMsPowerPoint();

    if (IsRegisteredFor(DEFAULT_HTML_EDITOR_FOR_IE))
        UnregisterAsDefaultHtmlEditorForInternetExplorer();

    if (IsRegisteredFor(HTML_EDITOR))
        UnregisterAsHtmlEditorForInternetExplorer();

    if (IsRegisteredFor(DEFAULT_SHELL_HTML_EDITOR))
        UnregisterAsDefaultShellHtmlEditor();

    if (m_RootKey->HasSubKey(PRIVATE_BACKUP_KEY_NAME))
        m_RootKey->DeleteSubKeyTree(PRIVATE_BACKUP_KEY_NAME);
}

void Registrar::RegisterForMsOfficeApplication(
    const std::wstring& FileExtension,
    const std::wstring& DocumentDisplayName,
    const std::wstring& DefaultIconEntry,
    const std::wstring& DefaultShellCommand,
    const std::wstring& ShellNewCommandDisplayName,
    const RegistrationContextInformation::OFFICE_APPLICATION eOfficeApp) const
{
    assert(m_RootKey.get());

    std::wstring ForwardKeyName = FORWARD_KEY_PREFIX + FileExtension;

    RegistryKey ForwardKey = m_RootKey->CreateSubKey(ForwardKeyName);
    RegistryValue RegVal(new RegistryValueImpl(std::wstring(DEFAULT_VALUE_NAME), DocumentDisplayName));
    ForwardKey->SetValue(RegVal);

    RegistryKey RegKey = ForwardKey->CreateSubKey(L"DefaultIcon");
    RegVal->SetValue(DefaultIconEntry);
    RegKey->SetValue(RegVal);

    RegistryKey RegKeyShell = ForwardKey->CreateSubKey(L"shell");
    RegVal->SetValue(DefaultShellCommand);
    RegKeyShell->SetValue(RegVal);

    RegKey = RegKeyShell->CreateSubKey(L"new");
    RegVal->SetValue(ShellNewCommandDisplayName);
    RegKey->SetValue(RegVal);

    RegKey = RegKey->CreateSubKey(L"command");
    RegVal->SetValue(m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::New, eOfficeApp));
    RegKey->SetValue(RegVal);

    RegKey = RegKeyShell->CreateSubKey(L"open\\command");
    RegVal->SetValue(m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::Open, eOfficeApp));
    RegKey->SetValue(RegVal);

    RegKey = RegKeyShell->CreateSubKey(L"print\\command");
    RegVal->SetValue(m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::Print, eOfficeApp));
    RegKey->SetValue(RegVal);

    RegKey = RegKeyShell->CreateSubKey(L"printto\\command");
    RegVal->SetValue(m_ContextInformation.GetOpenOfficeCommandline(RegistrationContextInformation::Printto, eOfficeApp));
    RegKey->SetValue(RegVal);

    // set the new forward key under the appropriate extension
    RegKey = m_RootKey->CreateSubKey(FileExtension);

    if (RegKey->HasValue(DEFAULT_VALUE_NAME))
    {
        RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);

        if (REG_SZ == RegVal->GetType())
        {
            std::wstring str = RegVal->GetDataAsUniString();
            if (!IsOpenOfficeRegisteredForMsApplication(str))
                ForwardKey->CopyValue(RegKey, DEFAULT_VALUE_NAME, BACKUP_VALUE_NAME);
        }
    }

    RegVal->SetValue(ForwardKeyName);
    RegKey->SetValue(RegVal);
}

void Registrar::UnregisterForMsOfficeApplication(const std::wstring& FileExtension) const
{
    std::wstring FwdRegKeyName = FORWARD_KEY_PREFIX + FileExtension;

    if (m_RootKey->HasSubKey(FileExtension))
    {
        RegistryKey RegKey = m_RootKey->OpenSubKey(FileExtension);

        if (RegKey->HasValue(DEFAULT_VALUE_NAME))
        {
            RegistryValue RegVal = RegKey->GetValue(DEFAULT_VALUE_NAME);
            if (REG_SZ == RegVal->GetType() &&
                IsOpenOfficeRegisteredForMsApplication(RegVal->GetDataAsUniString()))
            {
                RegistryKey FwdRegKey = m_RootKey->CreateSubKey(FwdRegKeyName);

                if (FwdRegKey->HasValue(BACKUP_VALUE_NAME))
                    RegKey->CopyValue(FwdRegKey, BACKUP_VALUE_NAME, DEFAULT_VALUE_NAME);
                else
                    RegKey->DeleteValue(DEFAULT_VALUE_NAME);
            }
        }
    }

    if (m_RootKey->HasSubKey(FwdRegKeyName))
        m_RootKey->DeleteSubKeyTree(FwdRegKeyName);
}

RegistryKey Registrar::GetRootKeyForDefHtmlEditorForIERegistration() const
{
    return WindowsRegistry().GetLocalMachineKey();
}

bool Registrar::IsOpenOfficeRegisteredForMsApplication(const std::wstring& DocumentExtensionDefValue) const
{
    return (std::wstring::npos != DocumentExtensionDefValue.find(FORWARD_KEY_PREFIX));
}
