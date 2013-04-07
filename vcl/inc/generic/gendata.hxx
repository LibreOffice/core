/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Michael Meeks <michael.meeks@suse.org> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _SV_GENDATA_HXX
#define _SV_GENDATA_HXX

#include <osl/socket.hxx>
#include <saldatabasic.hxx>

class SalGenericDisplay;

// Not the prettiest - but helpful for migrating old code ...
class SalDisplay;
class GtkSalDisplay;
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
    SalGenericData( SalGenericDataType t, SalInstance *pInstance ) : SalData(), m_eType( t ), m_pDisplay( NULL ) { m_pInstance = pInstance; SetSalData( this ); }
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

    // Not the prettiest - but helpful for migrating old code ...
    inline SalDisplay *GetSalDisplay() const
    {
        OSL_ASSERT( m_eType != SAL_DATA_GTK3 );
        return (SalDisplay *)GetDisplay();
    }
    inline GtkSalDisplay *GetGtkDisplay() const
    {
        return (GtkSalDisplay *)GetDisplay();
    }
};

inline SalGenericData * GetGenericData()
{
    return (SalGenericData *)ImplGetSVData()->mpSalData;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
