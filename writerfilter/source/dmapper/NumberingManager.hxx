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

#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_NUMBERINGMANAGER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_NUMBERINGMANAGER_HXX

#include "PropertyMap.hxx"

#include <DomainMapper.hxx>
#include "LoggedResources.hxx"

#include <editeng/numitem.hxx>

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

namespace writerfilter {
namespace dmapper {

class DomainMapper;
class StyleSheetEntry;


/** Class representing the numbering level properties.
 */
class ListLevel : public PropertyMap
{
    sal_Int32                                     m_nIStartAt;       //LN_CT_Lvl_start
    sal_Int32                                     m_nNFC;            //LN_CT_Lvl_numFmt
    sal_Int32                                     m_nJC;             //LN_JC
    sal_Int32                                     m_nFLegal;         //LN_FLEGAL
    sal_Int32                                     m_nFPrev;          //LN_FPREV
    sal_Int32                                     m_nFPrevSpace;     //LN_FPREVSPACE
    sal_Int32                                     m_nFWord6;         //LN_FWORD6
    sal_Int16                                     m_nXChFollow;      //LN_IXCHFOLLOW
    OUString                               m_sBulletChar;
    OUString                               m_sGraphicURL;
    css::uno::Reference<css::graphic::XGraphic> m_sGraphicBitmap;
    sal_Int32                                     m_nTabstop;
    std::shared_ptr< StyleSheetEntry >          m_pParaStyle;
    bool                                          m_outline;

public:

    typedef std::shared_ptr< ListLevel > Pointer;

    ListLevel() :
        m_nIStartAt(-1)
        ,m_nNFC(-1)
        ,m_nJC(-1)
        ,m_nFLegal(-1)
        ,m_nFPrev(-1)
        ,m_nFPrevSpace(-1)
        ,m_nFWord6(-1)
        ,m_nXChFollow(SvxNumberFormat::LISTTAB)
        ,m_nTabstop( 0 )
        ,m_outline(false)
        {}

    virtual ~ListLevel( ){ }

    // Setters for the import
    void SetValue( Id nId, sal_Int32 nValue );
    void SetBulletChar( const OUString& sValue ) { m_sBulletChar = sValue; };
    void SetGraphicURL( const OUString& sValue ) { m_sGraphicURL = sValue; };
    void SetGraphicBitmap(css::uno::Reference<css::graphic::XGraphic> const& sValue)
        { m_sGraphicBitmap = sValue; }
    void SetParaStyle( std::shared_ptr< StyleSheetEntry > pStyle );

    // Getters
    OUString GetBulletChar( ) { return m_sBulletChar; };
    std::shared_ptr< StyleSheetEntry > GetParaStyle( ) { return m_pParaStyle; };
    bool isOutlineNumbering() const { return m_outline; }

    // UNO mapping functions

    // rPrefix and rSuffix are out parameters
    static sal_Int16 GetParentNumbering( const OUString& sText, sal_Int16 nLevel,
        OUString& rPrefix, OUString& rSuffix );

    css::uno::Sequence<css::beans::PropertyValue> GetProperties();

    css::uno::Sequence<css::beans::PropertyValue> GetCharStyleProperties();
private:

    css::uno::Sequence<css::beans::PropertyValue> GetLevelProperties();

    void AddParaProperties(css::uno::Sequence<css::beans::PropertyValue>* pProps);
};

/// Represents a numbering picture bullet: an id and a graphic.
class NumPicBullet
{
public:
    typedef std::shared_ptr<NumPicBullet> Pointer;
    NumPicBullet();
    virtual ~NumPicBullet();

    void SetId(sal_Int32 nId);
    sal_Int32 GetId() { return m_nId;}
    void SetShape(css::uno::Reference<css::drawing::XShape> const& xShape);
    css::uno::Reference<css::drawing::XShape> GetShape() { return m_xShape; }
private:
    sal_Int32 m_nId;
    css::uno::Reference<css::drawing::XShape> m_xShape;
};

class AbstractListDef
{
private:
    sal_Int32                            m_nTmpl;

    // The ID member reflects either the abstractNumId or the numId
    // depending on the use of the class
    sal_Int32                            m_nId;

    // Properties of each level. This can also reflect the overridden
    // levels of a numbering.
    ::std::vector< ListLevel::Pointer >  m_aLevels;

    // Only used during the numberings import
    ListLevel::Pointer                         m_pCurrentLevel;

    // The style name linked to.
    ::rtl::OUString                      m_sNumStyleLink;

public:
    typedef std::shared_ptr< AbstractListDef > Pointer;

    AbstractListDef( );
    virtual ~AbstractListDef( );

    // Setters using during the import
    void SetId( sal_Int32 nId ) { m_nId = nId; };
    void SetValue( sal_uInt32 nSprmId, sal_Int32 nValue );

    // Accessors
    sal_Int32             GetId( ) { return m_nId; };

    sal_Int16             Size( ) { return sal_Int16( m_aLevels.size( ) ); };
    ListLevel::Pointer    GetLevel( sal_uInt16 nLvl );
    void                  AddLevel( );

    ListLevel::Pointer    GetCurrentLevel( ) { return m_pCurrentLevel; };

    virtual css::uno::Sequence< css::uno::Sequence<css::beans::PropertyValue> > GetPropertyValues();

    void                  SetNumStyleLink(const OUString& sValue) { m_sNumStyleLink = sValue; };
    OUString              GetNumStyleLink() { return m_sNumStyleLink; };
};

class ListDef : public AbstractListDef
{
private:
    // Pointer to the abstract numbering
    AbstractListDef::Pointer             m_pAbstractDef;

    // Cache for the UNO numbering rules
    css::uno::Reference< css::container::XIndexReplace > m_xNumRules;

public:
    typedef std::shared_ptr< ListDef > Pointer;

    ListDef( );
    virtual ~ListDef( );

    // Accessors
    void SetAbstractDefinition( AbstractListDef::Pointer pAbstract ) { m_pAbstractDef = pAbstract; };
    AbstractListDef::Pointer GetAbstractDefinition( ) { return m_pAbstractDef; };

    // Mapping functions
    static OUString GetStyleName( sal_Int32 nId );

    css::uno::Sequence< css::uno::Sequence<css::beans::PropertyValue> > GetPropertyValues() override;

    void CreateNumberingRules(DomainMapper& rDMapper, css::uno::Reference<css::lang::XMultiServiceFactory> const& xFactory);

    css::uno::Reference<css::container::XIndexReplace> GetNumberingRules() { return m_xNumRules; }

};

/** This class provides access to the defined numbering styles.
  */
class ListsManager :
    public LoggedProperties,
    public LoggedTable
{
private:

    DomainMapper&                                       m_rDMapper;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xFactory;

    // The numbering entries
    std::vector< NumPicBullet::Pointer >                m_aNumPicBullets;
    std::vector< AbstractListDef::Pointer >             m_aAbstractLists;
    std::vector< ListDef::Pointer >                     m_aLists;


    // These members are used for import only
    AbstractListDef::Pointer                            m_pCurrentDefinition;
    NumPicBullet::Pointer                               m_pCurrentNumPicBullet;
    bool                                                m_bIsLFOImport;

    AbstractListDef::Pointer    GetAbstractList( sal_Int32 nId );

    // Properties
    virtual void lcl_attribute( Id nName, Value & rVal ) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) override;

public:

    ListsManager(DomainMapper& rDMapper, const css::uno::Reference<css::lang::XMultiServiceFactory>& xFactory);
    virtual ~ListsManager();

    typedef std::shared_ptr< ListsManager >  Pointer;

    ListDef::Pointer        GetList( sal_Int32 nId );

    // Mapping methods
    void CreateNumberingRules( );

    // Dispose the NumPicBullets
    void DisposeNumPicBullets( );
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
