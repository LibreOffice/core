/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// UserRegistrar.h: Schnittstelle für die Klasse UserRegistrar.
//
//////////////////////////////////////////////////////////////////////

#ifndef _USERREGISTRAR_HXX_
#define _USERREGISTRAR_HXX_

#include "registrar.hxx"

class UserRegistrar : public Registrar
{
public:

    //###################################
    // Creation
    //###################################

    UserRegistrar(const RegistrationContextInformation& RegContext);

    //###################################
    // Command
    //###################################

    virtual void UnregisterAsHtmlEditorForInternetExplorer() const;

    virtual void RegisterAsDefaultShellHtmlEditor() const;
    virtual void UnregisterAsDefaultShellHtmlEditor() const;

protected:
    virtual void UnregisterForMsOfficeApplication(
        const std::wstring& FileExtension) const;

    virtual RegistryKey GetRootKeyForDefHtmlEditorForIERegistration() const;

private:

    /** Delete the privately created file associations
          for htm files if the keys are empty
    */
    void DeleteHtmFileAssociationKeys() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
