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

#ifndef _XMLOFF_SVXMLATTR_HXX
#define _XMLOFF_SVXMLATTR_HXX

#include <rtl/ustring.hxx>   //OUString

class SvXMLAttr {
private:
    sal_uInt16      aPrefixPos;
    OUString aLName;
    OUString aValue;
public:
    // Assuming OUString is well behaved, the default copy constructor and
    // assignment operator are fine.
    SvXMLAttr( const OUString& rLName,
               const OUString& rValue );
    SvXMLAttr( const sal_uInt16 nPos,
               const OUString& rLName,
               const OUString& rValue );
    bool operator== (const SvXMLAttr &rCmp) const;

    sal_uInt16       getPrefixPos() const;
    const OUString& getLName() const;
    const OUString& getValue() const;
};

#endif /* _XMLOFF_SVXMLATTR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
