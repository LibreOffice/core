/*************************************************************************
 *
 *  $RCSfile: xmlcnitm.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLCNITM_HXX
#define _XMLOFF_XMLCNITM_HXX

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

class SvXMLNamespaceMap;
namespace rtl { class OUString; }

//============================================================================

class SvXMLAttrContainerItem_Impl;

class SvXMLAttrContainerItem: public SfxPoolItem
{
    SvXMLAttrContainerItem_Impl *pImpl;

public:
    TYPEINFO();

    SvXMLAttrContainerItem( sal_uInt16 nWhich = 0 );
    SvXMLAttrContainerItem( const SvXMLAttrContainerItem& );
    virtual ~SvXMLAttrContainerItem();

    virtual int operator==( const SfxPoolItem& ) const;
    virtual int Compare( const SfxPoolItem &rWith ) const;

    virtual SfxItemPresentation GetPresentation(
                                SfxItemPresentation ePresentation,
                                SfxMapUnit eCoreMetric,
                                SfxMapUnit ePresentationMetric,
                                XubString &rText,
                                const International *pInternational = 0 ) const;

    virtual sal_uInt16 GetVersion( sal_uInt16 nFileFormatVersion ) const;

    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxPoolItem *Clone( SfxItemPool * = 0) const
    { return new SvXMLAttrContainerItem( *this ); }

    sal_Bool AddAttr( const ::rtl::OUString& rLName,
                          const ::rtl::OUString& rValue );
    sal_Bool AddAttr( const ::rtl::OUString& rPrefix,
                          const ::rtl::OUString& rNamespace,
                          const ::rtl::OUString& rLName,
                          const ::rtl::OUString& rValue );

    sal_uInt16 GetAttrCount() const;
    ::rtl::OUString GetAttrNamespace( sal_uInt16 i ) const;
    ::rtl::OUString GetAttrPrefix( sal_uInt16 i ) const;
    const ::rtl::OUString& GetAttrLName( sal_uInt16 i ) const;
    const ::rtl::OUString& GetAttrValue( sal_uInt16 i ) const;

    sal_uInt16 GetFirstNamespaceIndex() const;
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const;
    const ::rtl::OUString& GetNamespace( sal_uInt16 i ) const;
    const ::rtl::OUString& GetPrefix( sal_uInt16 i ) const;
};

#endif //  _XMLOFF_XMLCNITM_HXX

