/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_GENERIC_GENDATA_HXX
#define INCLUDED_VCL_INC_GENERIC_GENDATA_HXX

#include <osl/socket.hxx>

#include <saldatabasic.hxx>

class SalGenericDisplay;

enum SalGenericDataType { SAL_DATA_GTK, SAL_DATA_GTK3,
                          SAL_DATA_TDE3, SAL_DATA_KDE3, SAL_DATA_KDE4,
                          SAL_DATA_UNX, SAL_DATA_SVP,
                          SAL_DATA_ANDROID, SAL_DATA_IOS,
                          SAL_DATA_HEADLESS };

class VCL_DLLPUBLIC SalGenericData : public SalData
{
 protected:
    SalGenericDataType m_eType;
    SalGenericDisplay *m_pDisplay;
    // cached hostname to avoid slow lookup
    OUString      m_aHostname;
    // for transient storage of unicode strings eg. 'u123' by input methods
    OUString      m_aUnicodeEntry;
 public:
    SalGenericData( SalGenericDataType t, SalInstance *pInstance ) : SalData(), m_eType( t ), m_pDisplay( nullptr ) { m_pInstance = pInstance; SetSalData( this ); }
    virtual ~SalGenericData() {}
    virtual void Dispose() {}

    SalGenericDisplay *GetDisplay() const { return m_pDisplay; }
    void               SetDisplay( SalGenericDisplay *pDisp ) { m_pDisplay = pDisp; }

    const OUString& GetHostname()
    {
        if (m_aHostname.isEmpty())
            osl_getLocalHostname( &m_aHostname.pData );
        return m_aHostname;
    }
    OUString &GetUnicodeCommand()
    {
        return m_aUnicodeEntry;
    }
    inline SalGenericDataType GetType() const
    {
        return m_eType;
    }

    // Mostly useful for remote protocol backends
    virtual void ErrorTrapPush() = 0;
    virtual bool ErrorTrapPop( bool bIgnoreError = true ) = 0; // true on error
};

inline SalGenericData * GetGenericData()
{
    return static_cast<SalGenericData *>(ImplGetSVData()->mpSalData);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
