/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    ElementConfig( const rtl::OUString& rName, ElementConfigType rType ) : maName( rName ), mnType( rType ) {}
    ElementConfig( const rtl::OUString& rName ) : maName( rName ) {}
    ElementConfig( ElementConfigType rType ) : mnType( rType ) {}

    virtual rtl::OUString format( SvStream& rStream, sal_Size& nLength ) const;

    const rtl::OUString& getName() const { return maName; }
    ElementConfigType getType() const { return mnType; }

    static rtl::OUString dump_hex( SvStream& rStream, sal_Size& nLength );
    static rtl::OUString dump_byte( SvStream& rStream, sal_Size& nLength );
    static rtl::OUString dump_uint( SvStream& rStream, sal_Size& nLength );
    static rtl::OUString dump_unistring( SvStream& rStream, sal_Size& nLength );
    static rtl::OUString dump_float( SvStream& rStream, sal_Size& nLength );
private:
    rtl::OUString maName;
    ElementConfigType mnType;
};
typedef boost::shared_ptr< ElementConfig > ElementConfigPtr;
typedef std::list< ElementConfigPtr > ElementConfigList;

///////////////////////////////////////////////////////////////////////

class ElementValueConfig : public ElementConfig
{
public:
    ElementValueConfig( const rtl::OUString& rName, const rtl::OUString& rValue ) : ElementConfig( rName ), maValue( rValue ) {}

    const rtl::OUString& getValue() const { return maValue; }

private:
    rtl::OUString maValue;
};

///////////////////////////////////////////////////////////////////////

class ElementConfigContainer : public ElementConfig
{
public:
    ElementConfigContainer() : ElementConfig( ETC_CONTAINER ) {}
    ElementConfigContainer( const ::rtl::OUString& rName, ElementConfigType rType ) : ElementConfig( rName, rType ) {}
    ElementConfigContainer( const ::rtl::OUString& rName ) : ElementConfig( rName, ETC_CONTAINER ) {}
    ElementConfigContainer( ElementConfigType rType ) : ElementConfig( rType ) {}

    virtual rtl::OUString format( SvStream& rStream, sal_Size& nLength ) const;

    void addElementConfig( ElementConfigPtr p ) { maElementConfigList.push_back( p ); }

protected:
    ElementConfigList maElementConfigList;
};

///////////////////////////////////////////////////////////////////////

class CaseElementConfig : public ElementConfigContainer
{
public:
    CaseElementConfig( const rtl::OUString& rValue ) : maValue( rValue ) {}

    const rtl::OUString& getValue() const { return maValue; }

private:
    rtl::OUString maValue;
};

///////////////////////////////////////////////////////////////////////

class SwitchElementConfig : public ElementConfigContainer
{
public:
    SwitchElementConfig( ElementConfigType rType ) : ElementConfigContainer( rType ) {}

    virtual rtl::OUString format( SvStream& rStream, sal_Size& nLength ) const;
};

///////////////////////////////////////////////////////////////////////

class AtomConfig : public ElementConfigContainer
{
public:
    AtomConfig( const ::rtl::OUString& rName, bool bIsContainer ) : ElementConfigContainer( rName ), mbIsContainer( bIsContainer ) {}

    bool isContainer() const { return mbIsContainer; }

protected:
    bool mbIsContainer;
};

typedef std::map< UINT16, ElementConfigPtr > AtomConfigMap;

extern AtomConfigMap gAtomConfigMap;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
