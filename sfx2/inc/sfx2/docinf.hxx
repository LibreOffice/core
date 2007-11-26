/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docinf.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 15:34:21 $
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
#ifndef _SFXDOCINF_HXX
#define _SFXDOCINF_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/document/XDocumentInfo.hpp>

#ifndef _ERRCODE_HXX //autogen wg. ErrCode
#include <tools/errcode.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif

#include <hash_map>
#include <svtools/itemprop.hxx>

class SfxObjectShell;
class GDIMetaFile;
class SotStorage;
class SfxDocumentInfo_Impl;

const BYTE MAXDOCUSERKEYS  = 4;

class SFX2_DLLPUBLIC SfxDocumentInfo
{
    SfxDocumentInfo_Impl* pImp;

    SAL_DLLPRIVATE DateTime DATETIME_PROP(sal_Int32 nHandle) const;
    SAL_DLLPRIVATE BOOL BOOL_PROP(sal_Int32 nHandle) const;
    SAL_DLLPRIVATE sal_Int32 INT_PROP(sal_Int32 nHandle) const;
    SAL_DLLPRIVATE String STRING_PROP(sal_Int32 nHandle) const;
    SAL_DLLPRIVATE void SET_PROP(sal_Int32 nHandle, const com::sun::star::uno::Any& aValue );

public:

    SfxDocumentInfo( SfxObjectShell* pDoc=0);

    // deep copy
    SfxDocumentInfo( const SfxDocumentInfo& );

    // shallow copy
    SfxDocumentInfo( const com::sun::star::uno::Reference < com::sun::star::document::XDocumentInfo >& );

    ~SfxDocumentInfo();

    const SfxDocumentInfo& operator=( const SfxDocumentInfo& );
    int operator==( const SfxDocumentInfo& ) const;

    com::sun::star::uno::Reference < com::sun::star::document::XDocumentInfo > GetInfo() const;

    sal_uInt32 LoadPropertySet( SotStorage* pStor );
    sal_Bool SavePropertySet( SotStorage* pStor, GDIMetaFile* pThumb ) const;

    BOOL IsReloadEnabled() const;
    sal_Int16 GetDocumentNumber() const;
    sal_Int32 GetReloadDelay() const;
    sal_Int32 GetTime() const;
    String GetDefaultTarget() const;
    String GetReloadURL() const;
    String GetTemplateName() const;
    String GetTemplateFileName() const;
    String GetTitle()   const;
    String GetTheme()   const;
    String GetComment() const;
    String GetKeywords()const;
    String GetMimeType() const;
    String GetAuthor() const;
    DateTime GetCreationDate() const;
    String GetModificationAuthor() const;
    DateTime GetModificationDate() const;
    String GetPrintedBy() const;
    DateTime GetPrintDate() const;
    DateTime GetTemplateDate() const;

    void EnableReload( BOOL bEnable );
    void SetDocumentNumber(sal_Int16 nNo);
    void SetReloadDelay( sal_Int32 nDelay );
    void SetTime(sal_Int32 l);
    void SetDefaultTarget( const String& rString );
    void SetReloadURL( const String& rString );
    void SetTemplateName( const String& rName );
    void SetTemplateFileName( const String& rFileName );
    void SetTitle( const String& rVal );
    void SetTheme( const String& rVal );
    void SetComment( const String& rVal );
    void SetKeywords( const String& rVal );
    void SetMimeType( const String& rVal );
    void SetAuthor( const String& rAuthor );
    void SetModificationAuthor( const String& rChangedBy );
    void SetPrintedBy( const String& rPrintedBy );
    void SetCreationDate(const DateTime& rDate);
    void SetModificationDate(const DateTime& rDate);
    void SetPrintDate(const DateTime& rDate);
    void SetTemplateDate(const DateTime& rDate);
    void IncDocumentNumber();
    void SetCreated( const String& rAuthor );
    void SetChanged( const String& rName );
    void SetPrinted( const String& rName );

    USHORT GetUserKeyCount() const;
    String GetUserKeyTitle(USHORT n) const;
    String GetUserKeyWord(USHORT n) const;
    void   SetUserKey( const String& rName, const String& rValue, USHORT n );
    void   SetUserKeyWord( const String& rValue, USHORT n );
    void   SetUserKeyTitle( const String& rValue, USHORT n );

    void ClearTemplateInformation();
    void DeleteUserData( const String* pAuthor=0 );
    void ResetUserData( const String& rAuthor );
    void Clear();
    void ResetFromTemplate( const String& rTemplateName, const String& rFileName );

    // --> PB 2004-08-23 #i33095#
    sal_Bool        IsLoadReadonly() const;
    void            SetLoadReadonly( sal_Bool _bReadonly );

    BOOL LoadFromBinaryFormat( SvStream& rStream );
    BOOL SaveToBinaryFormat( SvStream& rStream ) const;
    BOOL LoadFromBinaryFormat( SotStorage* rStream );
    BOOL SaveToBinaryFormat( SotStorage* rStream ) const;

    BOOL InsertCustomProperty(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue);
    com::sun::star::uno::Sequence < ::rtl::OUString > GetCustomPropertyNames() const;
};

DECL_PTRHINT(SFX2_DLLPUBLIC, SfxDocumentInfoHint, SfxDocumentInfo);

#endif

