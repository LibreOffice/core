/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EDITENG_XMLCNITM_HXX
#define INCLUDED_EDITENG_XMLCNITM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLNamespaceMap;



class SvXMLAttrContainerData;

class EDITENG_DLLPUBLIC SvXMLAttrContainerItem: public SfxPoolItem
{
    SvXMLAttrContainerData  *pImpl;

public:
    TYPEINFO_OVERRIDE();

    SvXMLAttrContainerItem( sal_uInt16 nWhich = 0 );
    SvXMLAttrContainerItem( const SvXMLAttrContainerItem& );
    virtual ~SvXMLAttrContainerItem();

    virtual bool operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation(
                                SfxItemPresentation ePresentation,
                                SfxMapUnit eCoreMetric,
                                SfxMapUnit ePresentationMetric,
                                OUString &rText,
                                const IntlWrapper *pIntlWrapper = 0 ) const override;

    virtual sal_uInt16 GetVersion( sal_uInt16 nFileFormatVersion ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual SfxPoolItem *Clone( SfxItemPool * = 0) const override
    { return new SvXMLAttrContainerItem( *this ); }

    bool AddAttr( const OUString& rLName,
                          const OUString& rValue );
    bool AddAttr( const OUString& rPrefix,
                          const OUString& rNamespace,
                          const OUString& rLName,
                          const OUString& rValue );

    sal_uInt16 GetAttrCount() const;
    OUString GetAttrNamespace( sal_uInt16 i ) const;
    OUString GetAttrPrefix( sal_uInt16 i ) const;
    const OUString& GetAttrLName( sal_uInt16 i ) const;
    const OUString& GetAttrValue( sal_uInt16 i ) const;

    sal_uInt16 GetFirstNamespaceIndex() const;
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const;
    const OUString& GetNamespace( sal_uInt16 i ) const;
    const OUString& GetPrefix( sal_uInt16 i ) const;
};

#endif // INCLUDED_EDITENG_XMLCNITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
