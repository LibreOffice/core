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
 * The Initial Developer of the Original Code is
 *       Nigel Hawkins  - n.hawkins@gmx.com
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SVXMLATTRCOLLECTION_HXX_
#define SVXMLATTRCOLLECTION_HXX_

#include "SvXMLAttr.hxx"

#include <xmloff/nmspmap.hxx>  //SvXMLNamespaceMap
#include <rtl/ustring.hxx>     //OUString
#include <sal/types.h>         //sal_uInt16 and sal_Bool
#include <vector>

class SvXMLAttrCollection
{
public:
    SvXMLNamespaceMap      aNamespaceMap;
    std::vector<SvXMLAttr> aAttrs;

    bool operator==(const SvXMLAttrCollection &rCmp) const;
    sal_Bool AddAttr( const OUString& rLName,
                      const OUString& rValue );
    sal_Bool AddAttr( const OUString& rPrefix,
                      const OUString& rNamespace,
                      const OUString& rLName,
                      const OUString& rValue );
    sal_Bool AddAttr( const OUString& rPrefix,
                      const OUString& rLName,
                      const OUString& rValue );

    sal_Bool SetAt( size_t i,
                    const OUString& rLName,
                    const OUString& rValue );
    sal_Bool SetAt( size_t i,
                    const OUString& rPrefix,
                    const OUString& rNamespace,
                    const OUString& rLName,
                    const OUString& rValue );
    sal_Bool SetAt( size_t i,
                    const OUString& rPrefix,
                    const OUString& rLName,
                    const OUString& rValue );

    void Remove( size_t i );

    size_t GetAttrCount() const;
    const OUString& GetAttrLName(size_t i) const;
    const OUString& GetAttrValue(size_t i) const;
    const OUString GetAttrNamespace( size_t i ) const;
    const OUString GetAttrPrefix( size_t i ) const;
    const OUString& GetNamespace( sal_uInt16 i ) const;
    const OUString& GetPrefix( sal_uInt16 i ) const;
    sal_uInt16 GetFirstNamespaceIndex() const;
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const;

private:
    sal_uInt16 GetPrefixPos( size_t i ) const;
};

#endif /* SVXMLATTRCOLLECTION_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

