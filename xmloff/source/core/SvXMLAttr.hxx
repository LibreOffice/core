/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
