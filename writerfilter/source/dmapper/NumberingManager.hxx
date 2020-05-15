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

#include "DomainMapper.hxx"
#include "LoggedResources.hxx"
#include "StyleSheetTable.hxx"

#include <editeng/numitem.hxx>

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/awt/XBitmap.hpp>

namespace writerfilter {
namespace dmapper {

class DomainMapper;
class StyleSheetEntry;


/** Class representing the numbering level properties.
 */
class ListLevel : public PropertyMap
{
    sal_Int32                                     m_nIStartAt;       //LN_CT_Lvl_start
    sal_Int32                                     m_nStartOverride;
    sal_Int32                                     m_nNFC;            //LN_CT_Lvl_numFmt
    /// LN_CT_NumFmt_format, in case m_nNFC is custom.
    OUString m_aCustomNumberFormat;
    sal_Int16                                     m_nXChFollow;      //LN_IXCHFOLLOW
    OUString                               m_sBulletChar;
    css::awt::Size                         m_aGraphicSize;
    css::uno::Reference<css::awt::XBitmap> m_xGraphicBitmap;
    sal_Int32                                     m_nTabstop;
    tools::SvRef< StyleSheetEntry >          m_pParaStyle;
    bool                                          m_outline;
    bool m_bHasValues = false;

public:

    typedef tools::SvRef< ListLevel > Pointer;

    ListLevel() :
        m_nIStartAt(-1)
        ,m_nStartOverride(-1)
        ,m_nNFC(-1)
        ,m_nXChFollow(SvxNumberFormat::LISTTAB)
        ,m_nTabstop( 0 )
        ,m_outline(false)
        {}

    // Setters for the import
    void SetValue( Id nId, sal_Int32 nValue );
    void SetCustomNumberFormat(const OUString& rValue);
    void SetBulletChar( const OUString& sValue ) { m_sBulletChar = sValue; };
    void SetGraphicSize( const css::awt::Size& aValue ) { m_aGraphicSize = aValue; };

    void SetGraphicBitmap(css::uno::Reference<css::awt::XBitmap> const& xGraphicBitmap)
        { m_xGraphicBitmap = xGraphicBitmap; }
    void SetParaStyle( const tools::SvRef< StyleSheetEntry >& pStyle );

    // Getters
    const OUString& GetBulletChar( ) const { return m_sBulletChar; };
    const tools::SvRef< StyleSheetEntry >& GetParaStyle( ) const { return m_pParaStyle; };
    bool isOutlineNumbering() const { return m_outline; }
    sal_Int32 GetStartOverride() const { return m_nStartOverride; };
    /// Determines if SetValue() was called at least once.
    bool HasValues() const;

    // UNO mapping functions
    css::uno::Sequence<css::beans::PropertyValue> GetProperties(bool bDefaults);

    css::uno::Sequence<css::beans::PropertyValue> GetCharStyleProperties();
private:

    css::uno::Sequence<css::beans::PropertyValue> GetLevelProperties(bool bDefaults);

    void AddParaProperties(css::uno::Sequence<css::beans::PropertyValue>* pProps);
};

/// Represents a numbering picture bullet: an id and a graphic.
class NumPicBullet final : public virtual SvRefBase
{
public:
    typedef tools::SvRef<NumPicBullet> Pointer;
    NumPicBullet();
    ~NumPicBullet() override;

    void SetId(sal_Int32 nId);
    sal_Int32 GetId() const { return m_nId;}
    void SetShape(css::uno::Reference<css::drawing::XShape> const& xShape);
    const css::uno::Reference<css::drawing::XShape>& GetShape() const { return m_xShape; }
private:
    sal_Int32 m_nId;
    css::uno::Reference<css::drawing::XShape> m_xShape;
};

class AbstractListDef : public virtual SvRefBase
{
private:
    // The ID member reflects either the abstractNumId or the numId
    // depending on the use of the class
    sal_Int32                            m_nId;

    // Properties of each level. This can also reflect the overridden
    // levels of a numbering.
    ::std::vector< ListLevel::Pointer >  m_aLevels;

    // Only used during the numbering import
    ListLevel::Pointer                         m_pCurrentLevel;

    // The style name linked to.
    OUString                      m_sNumStyleLink;

    // This abstract numbering is a base definition for this style
    OUString                      m_sStyleLink;

    /// list id to use for all derived numbering definitions
    std::optional<OUString> m_oListId;

public:
    typedef tools::SvRef< AbstractListDef > Pointer;

    AbstractListDef( );
    virtual ~AbstractListDef( ) override;

    // Setters using during the import
    void SetId( sal_Int32 nId ) { m_nId = nId; };
    static void SetValue( sal_uInt32 nSprmId );

    // Accessors
    sal_Int32             GetId( ) const { return m_nId; };

    sal_Int16             Size( ) { return sal_Int16( m_aLevels.size( ) ); };
    ListLevel::Pointer    GetLevel( sal_uInt16 nLvl );
    void                  AddLevel( );

    const ListLevel::Pointer&  GetCurrentLevel( ) const { return m_pCurrentLevel; };

    css::uno::Sequence< css::uno::Sequence<css::beans::PropertyValue> > GetPropertyValues(bool bDefaults);

    void                  SetNumStyleLink(const OUString& sValue) { m_sNumStyleLink = sValue; };
    const OUString&       GetNumStyleLink() const { return m_sNumStyleLink; };

    void                  SetStyleLink(const OUString& sValue) { m_sStyleLink = sValue; };
    const OUString&       GetStyleLink() const { return m_sStyleLink; };

    const OUString& MapListId(OUString const& rId);
    bool isOutlineNumbering( sal_uInt16 nLvl ) { return GetLevel(nLvl) && GetLevel(nLvl)->isOutlineNumbering(); }
};

class ListDef : public AbstractListDef
{
private:
    // Pointer to the abstract numbering
    AbstractListDef::Pointer             m_pAbstractDef;

    // Cache for the UNO numbering rules
    css::uno::Reference< css::container::XIndexReplace > m_xNumRules;

    /// mapped list style name
    OUString m_StyleName;

public:
    typedef tools::SvRef< ListDef > Pointer;

    ListDef( );
    virtual ~ListDef( ) override;

    // Accessors
    void SetAbstractDefinition( AbstractListDef::Pointer pAbstract ) { m_pAbstractDef = pAbstract; };
    const AbstractListDef::Pointer& GetAbstractDefinition( ) const { return m_pAbstractDef; };

    // Mapping functions
    OUString GetStyleName() const { return m_StyleName; };
    OUString GetStyleName(sal_Int32 nId, css::uno::Reference<css::container::XNameContainer> const& xStyles);

    css::uno::Sequence< css::uno::Sequence<css::beans::PropertyValue> > GetMergedPropertyValues();

    void CreateNumberingRules(DomainMapper& rDMapper, css::uno::Reference<css::lang::XMultiServiceFactory> const& xFactory);

    const css::uno::Reference<css::container::XIndexReplace>& GetNumberingRules() const { return m_xNumRules; }

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

    AbstractListDef::Pointer    GetAbstractList( sal_Int32 nId );

    // Properties
    virtual void lcl_attribute( Id nName, Value & rVal ) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    // Table
    virtual void lcl_entry(writerfilter::Reference<Properties>::Pointer_t ref) override;

public:

    ListsManager(DomainMapper& rDMapper, const css::uno::Reference<css::lang::XMultiServiceFactory>& xFactory);
    virtual ~ListsManager() override;

    typedef tools::SvRef< ListsManager >  Pointer;

    ListDef::Pointer        GetList( sal_Int32 nId );

    // Mapping methods
    void CreateNumberingRules( );

    // Dispose the NumPicBullets
    void DisposeNumPicBullets( );
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
