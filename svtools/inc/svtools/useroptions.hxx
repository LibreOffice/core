/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: useroptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:42:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#define INCLUDED_SVTOOLS_USEROPTIONS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

// define ----------------------------------------------------------------

#define USER_OPT_CITY               ((USHORT)0)
#define USER_OPT_COMPANY            ((USHORT)1)
#define USER_OPT_COUNTRY            ((USHORT)2)
#define USER_OPT_EMAIL              ((USHORT)3)
#define USER_OPT_FAX                ((USHORT)4)
#define USER_OPT_FIRSTNAME          ((USHORT)5)
#define USER_OPT_LASTNAME           ((USHORT)6)
#define USER_OPT_POSITION           ((USHORT)7)
#define USER_OPT_STATE              ((USHORT)8)
#define USER_OPT_STREET             ((USHORT)9)
#define USER_OPT_TELEPHONEHOME      ((USHORT)10)
#define USER_OPT_TELEPHONEWORK      ((USHORT)11)
#define USER_OPT_TITLE              ((USHORT)12)
#define USER_OPT_ID                 ((USHORT)13)
#define USER_OPT_ZIP                ((USHORT)14)
#define USER_OPT_FATHERSNAME        ((USHORT)15)
#define USER_OPT_APARTMENT          ((USHORT)16)

// class SvtUserOptions --------------------------------------------------

class SvtUserOptions_Impl;

class SVT_DLLPUBLIC SvtUserOptions:
    public svt::detail::Options, public SfxBroadcaster, public SfxListener
{
private:
    SvtUserOptions_Impl*    pImp;

public:
    SvtUserOptions();
    virtual ~SvtUserOptions();

    static ::osl::Mutex&    GetInitMutex();

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // get the address token
    const String&   GetCompany() const;
    const String&   GetFirstName() const;
    const String&   GetLastName() const;
    const String&   GetID() const;
    const String&   GetStreet() const;
    const String&   GetCity() const;
    const String&   GetState() const;
    const String&   GetZip() const;
    const String&   GetCountry() const;
    const String&   GetPosition() const;
    const String&   GetTitle() const;
    const String&   GetTelephoneHome() const;
    const String&   GetTelephoneWork() const;
    const String&   GetFax() const;
    const String&   GetEmail() const;
    const String&   GetCustomerNumber() const;
    const String&   GetFathersName() const;
    const String&   GetApartment() const;

    const String&   GetFullName() const;
    const String&   GetLocale() const;

    // set the address token
    void            SetCompany( const String& rNewToken );
    void            SetFirstName( const String& rNewToken );
    void            SetLastName( const String& rNewToken );
    void            SetID( const String& rNewToken );
    void            SetStreet( const String& rNewToken );
    void            SetCity( const String& rNewToken );
    void            SetState( const String& rNewToken );
    void            SetZip( const String& rNewToken );
    void            SetCountry( const String& rNewToken );
    void            SetPosition( const String& rNewToken );
    void            SetTitle( const String& rNewToken );
    void            SetTelephoneHome( const String& rNewToken );
    void            SetTelephoneWork( const String& rNewToken );
    void            SetFax( const String& rNewToken );
    void            SetEmail( const String& rNewToken );
    void            SetCustomerNumber( const String& rNewToken );
    void            SetFathersName( const String& rNewToken );
    void            SetApartment( const String& rNewToken );

    sal_Bool        IsTokenReadonly( USHORT nToken ) const;
    const String&   GetToken(USHORT nToken) const;
};

#endif // #ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX

