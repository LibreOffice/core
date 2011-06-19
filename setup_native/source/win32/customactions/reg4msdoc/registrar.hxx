/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// Registrar.h: Schnittstelle f�r die Klasse Registrar.
//
//////////////////////////////////////////////////////////////////////

#ifndef _REGISTRAR_HXX_
#define _REGISTRAR_HXX_

#include "registry.hxx"
#include "registrationcontextinformation.hxx"

#include "constants.hxx"

class Registrar
{
public:

    Registrar(const RegistrationContextInformation& RegContext);

    virtual ~Registrar();

    bool IsRegisteredFor(int State) const;

    virtual void RegisterForMsWord() const;
    virtual void UnregisterForMsWord() const;
    virtual bool QueryPreselectMsWordRegistration() const;

    virtual void RegisterForMsExcel() const;
    virtual void UnregisterForMsExcel() const;
    virtual bool QueryPreselectMsExcelRegistration() const;

    virtual void RegisterForMsPowerPoint() const;
    virtual void UnregisterForMsPowerPoint() const;
    virtual bool QueryPreselectMsPowerPointRegistration() const;

    virtual void RegisterAsHtmlEditorForInternetExplorer() const;
    virtual void UnregisterAsHtmlEditorForInternetExplorer() const;

    virtual void RegisterAsDefaultHtmlEditorForInternetExplorer() const;
    virtual void UnregisterAsDefaultHtmlEditorForInternetExplorer() const;

    virtual void RegisterAsDefaultShellHtmlEditor() const;
    virtual void UnregisterAsDefaultShellHtmlEditor() const;

    /** Restore the last registration state (necessary for
        Setup repair)
    */
    virtual void RepairRegistrationState() const;

    /** Unregisters all and delete all Registry keys we have written
    */
    virtual void UnregisterAllAndCleanUpRegistry() const;

protected:

    virtual void RegisterForMsOfficeApplication(
        const std::wstring& FileExtension,
        const std::wstring& DocumentDisplayName,
        const std::wstring& DefaultIconEntry,
        const std::wstring& DefaultShellCommand,
        const std::wstring& ShellNewCommandDisplayName,
        const RegistrationContextInformation::OFFICE_APPLICATION eOfficeApp) const;

    virtual void UnregisterForMsOfficeApplication(
        const std::wstring& FileExtension) const;

    virtual RegistryKey GetRootKeyForDefHtmlEditorForIERegistration() const;

    void SaveRegisteredFor(int State) const;
    void SaveNotRegisteredFor(int State) const;

    int GetRegisterState() const;
    void SetRegisterState(int NewState) const;

    virtual bool QueryPreselectForMsApplication(const std::wstring& file_extension) const;

    /** A helper function (for readability) returns true if OpenOffice is already
          registered for a MS application

          @param DocumentExtensionDefValue
                      The default value of the appropriate document extension Registry key
    */
    bool IsOpenOfficeRegisteredForMsApplication(const std::wstring& DocumentExtensionDefValue) const;

protected:
    const RegistrationContextInformation&   m_ContextInformation;

    const std::wstring FORWARD_KEY_PREFIX;
    const std::wstring DEFAULT_VALUE_NAME;
    const std::wstring BACKUP_VALUE_NAME;
    const std::wstring PRIVATE_BACKUP_KEY_NAME;
    const std::wstring REGISTRATION_STATE;

    RegistryKey  m_RootKey;

// prevent copy/assignment
private:
    Registrar(const Registrar&);
    Registrar& operator=(const Registrar&);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
