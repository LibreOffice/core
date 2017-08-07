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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAOPTIONS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAOPTIONS_HXX

#include <ooo/vba/word/XOptions.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbapropvalue.hxx>
#include <comphelper/processfactory.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XOptions > SwVbaOptions_BASE;

class SwVbaOptions : public SwVbaOptions_BASE,
                    public PropListener
{
private:
    OUString msDefaultFilePath;
public:
    explicit SwVbaOptions( css::uno::Reference< css::uno::XComponentContext > const & m_xContext );
    virtual ~SwVbaOptions() override;

    // Attributes
    virtual ::sal_Int32 SAL_CALL getDefaultBorderLineStyle() override;
    virtual void SAL_CALL setDefaultBorderLineStyle( ::sal_Int32 _defaultborderlinestyle ) override;
    virtual ::sal_Int32 SAL_CALL getDefaultBorderLineWidth() override;
    virtual void SAL_CALL setDefaultBorderLineWidth( ::sal_Int32 _defaultborderlinewidth ) override;
    virtual ::sal_Int32 SAL_CALL getDefaultBorderColorIndex() override;
    virtual void SAL_CALL setDefaultBorderColorIndex( ::sal_Int32 _defaultbordercolorindex ) override;
    virtual sal_Bool SAL_CALL getReplaceSelection() override;
    virtual void SAL_CALL setReplaceSelection( sal_Bool _replaceselection ) override;
    virtual sal_Bool SAL_CALL getMapPaperSize() override;
    virtual void SAL_CALL setMapPaperSize( sal_Bool _mappapersize ) override;
    virtual sal_Bool SAL_CALL getAutoFormatAsYouTypeApplyHeadings() override;
    virtual void SAL_CALL setAutoFormatAsYouTypeApplyHeadings( sal_Bool _autoformatasyoutypeapplyheadings ) override;
    virtual sal_Bool SAL_CALL getAutoFormatAsYouTypeApplyBulletedLists() override;
    virtual void SAL_CALL setAutoFormatAsYouTypeApplyBulletedLists( sal_Bool _autoformatasyoutypeapplybulletedlists ) override;
    virtual sal_Bool SAL_CALL getAutoFormatAsYouTypeApplyNumberedLists() override;
    virtual void SAL_CALL setAutoFormatAsYouTypeApplyNumberedLists( sal_Bool _autoformatasyoutypeapplynumberedlists ) override;
    virtual sal_Bool SAL_CALL getAutoFormatAsYouTypeFormatListItemBeginning() override;
    virtual void SAL_CALL setAutoFormatAsYouTypeFormatListItemBeginning( sal_Bool _autoformatasyoutypeformatlistitembeginning ) override;
    virtual sal_Bool SAL_CALL getAutoFormatAsYouTypeDefineStyles() override;
    virtual void SAL_CALL setAutoFormatAsYouTypeDefineStyles( sal_Bool _autoformatasyoutypedefinestyles ) override;
    virtual sal_Bool SAL_CALL getAutoFormatApplyHeadings() override;
    virtual void SAL_CALL setAutoFormatApplyHeadings( sal_Bool _autoformatapplyheadings ) override;
    virtual sal_Bool SAL_CALL getAutoFormatApplyLists() override;
    virtual void SAL_CALL setAutoFormatApplyLists( sal_Bool _autoformatapplylists ) override;
    virtual sal_Bool SAL_CALL getAutoFormatApplyBulletedLists() override;
    virtual void SAL_CALL setAutoFormatApplyBulletedLists( sal_Bool _autoformatapplybulletedlists ) override;

    // Methods
    virtual css::uno::Any SAL_CALL DefaultFilePath( sal_Int32 _path ) override;

    //PropListener
    virtual void setValueEvent( const css::uno::Any& value ) override;
    virtual css::uno::Any getValueEvent() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
