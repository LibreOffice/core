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

#ifndef _XMLCONFIG_HXX_
#define _XMLCONFIG_HXX_

#include <map>
#include <list>
#include <boost/shared_ptr.hpp>

///////////////////////////////////////////////////////////////////////

enum ElementConfigType { ECT_HEXDUMP, ECT_BYTE, ECT_UINT, ECT_UNISTRING, ETC_FLOAT, ETC_CONTAINER };

class ElementConfig
{
public:
    ElementConfig() : mnType( ECT_HEXDUMP ) {}
    ElementConfig( const OUString& rName, ElementConfigType rType ) : maName( rName ), mnType( rType ) {}
    ElementConfig( const OUString& rName ) : maName( rName ) {}
    ElementConfig( ElementConfigType rType ) : mnType( rType ) {}

    virtual OUString format( SvStream& rStream, sal_Size& nLength ) const;

    const OUString& getName() const { return maName; }
    ElementConfigType getType() const { return mnType; }

    static OUString dump_hex( SvStream& rStream, sal_Size& nLength );
    static OUString dump_byte( SvStream& rStream, sal_Size& nLength );
    static OUString dump_uint( SvStream& rStream, sal_Size& nLength );
    static OUString dump_unistring( SvStream& rStream, sal_Size& nLength );
    static OUString dump_float( SvStream& rStream, sal_Size& nLength );
private:
    OUString maName;
    ElementConfigType mnType;
};
typedef boost::shared_ptr< ElementConfig > ElementConfigPtr;
typedef std::list< ElementConfigPtr > ElementConfigList;

///////////////////////////////////////////////////////////////////////

class ElementValueConfig : public ElementConfig
{
public:
    ElementValueConfig( const OUString& rName, const OUString& rValue ) : ElementConfig( rName ), maValue( rValue ) {}

    const OUString& getValue() const { return maValue; }

private:
    OUString maValue;
};

///////////////////////////////////////////////////////////////////////

class ElementConfigContainer : public ElementConfig
{
public:
    ElementConfigContainer() : ElementConfig( ETC_CONTAINER ) {}
    ElementConfigContainer( const OUString& rName, ElementConfigType rType ) : ElementConfig( rName, rType ) {}
    ElementConfigContainer( const OUString& rName ) : ElementConfig( rName, ETC_CONTAINER ) {}
    ElementConfigContainer( ElementConfigType rType ) : ElementConfig( rType ) {}

    virtual OUString format( SvStream& rStream, sal_Size& nLength ) const;

    void addElementConfig( ElementConfigPtr p ) { maElementConfigList.push_back( p ); }

protected:
    ElementConfigList maElementConfigList;
};

///////////////////////////////////////////////////////////////////////

class CaseElementConfig : public ElementConfigContainer
{
public:
    CaseElementConfig( const OUString& rValue ) : maValue( rValue ) {}

    const OUString& getValue() const { return maValue; }

private:
    OUString maValue;
};

///////////////////////////////////////////////////////////////////////

class SwitchElementConfig : public ElementConfigContainer
{
public:
    SwitchElementConfig( ElementConfigType rType ) : ElementConfigContainer( rType ) {}

    virtual OUString format( SvStream& rStream, sal_Size& nLength ) const;
};

///////////////////////////////////////////////////////////////////////

class AtomConfig : public ElementConfigContainer
{
public:
    AtomConfig( const OUString& rName, bool bIsContainer ) : ElementConfigContainer( rName ), mbIsContainer( bIsContainer ) {}

    bool isContainer() const { return mbIsContainer; }

protected:
    bool mbIsContainer;
};

typedef std::map< UINT16, ElementConfigPtr > AtomConfigMap;

extern AtomConfigMap gAtomConfigMap;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
