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
#ifndef INCLUDED_SVX_SOURCE_UNODRAW_SHAPEIMPL_HXX
#define INCLUDED_SVX_SOURCE_UNODRAW_SHAPEIMPL_HXX

#include <svx/unoshape.hxx>
class SvxShapeCaption : public SvxShapeText
{
public:
    explicit SvxShapeCaption(SdrObject* pObj);
    virtual ~SvxShapeCaption() throw() override;
};
class SvxPluginShape : public SvxOle2Shape
{
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) override;

public:
    explicit SvxPluginShape(SdrObject* pObj, OUString referer);
    virtual ~SvxPluginShape() throw() override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    using SvxUnoTextRangeBase::setPropertyValue;

    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage ) override;
};

class SvxAppletShape : public SvxOle2Shape
{
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) override;

public:
    explicit SvxAppletShape(SdrObject* pObj, OUString referer);
    virtual ~SvxAppletShape() throw() override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    using SvxUnoTextRangeBase::setPropertyValue;

    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage ) override;
};

class SvxFrameShape : public SvxOle2Shape
{
private:
    OUString m_sInitialFrameURL;
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl(const OUString& rName, const SfxItemPropertySimpleEntry* pProperty,
        css::uno::Any& rValue) override;

public:
    explicit SvxFrameShape(SdrObject* pObj, OUString referer);
    virtual ~SvxFrameShape() throw() override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    using SvxUnoTextRangeBase::setPropertyValue;

    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage ) override;

    virtual OUString GetAndClearInitialFrameURL() override;
};


class SvxTableShape : public SvxShape
{
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) override;

    virtual void lock() override;
    virtual void unlock() override;

public:

    explicit SvxTableShape(SdrObject* pObj);
    virtual ~SvxTableShape() throw() override;
};

SvxUnoPropertyMapProvider& getSvxMapProvider();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
