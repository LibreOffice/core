/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_VBAHELPER_VBAFONTBASE_HXX
#define INCLUDED_VBAHELPER_VBAFONTBASE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <ooo/vba/XFontBase.hpp>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com::sun::star {
    namespace beans { class XPropertySet; }
    namespace container { class XIndexAccess; }
}
namespace cpo::uno { class XComponentContext; }

namespace ooo::vba {
    class XHelperInterface;
}

typedef  InheritedHelperInterfaceWeakImpl< ov::XFontBase > VbaFontBase_BASE;

class VBAHELPER_DLLPUBLIC VbaFontBase : public VbaFontBase_BASE
{
public:
    enum Component { WORD, EXCEL };

protected:
    css::uno::Reference< css::beans::XPropertySet > mxFont;
    css::uno::Reference< css::container::XIndexAccess > mxPalette;
    Component meWhich;
    bool mbFormControl;

public:
    // use local constants there is no need to expose these constants
    // externally. Looking at the Format->Character dialog it seem that
    // these may in fact even be calculated. Leave hardcoded for now
    // #FIXME #TBD investigate the code for dialog mentioned above

    // The font baseline is not specified.
    static const short NORMAL = 0;

    // specifies a superscripted.
    static const short SUPERSCRIPT = 33;

    // specifies a subscripted.
    static const short SUBSCRIPT = -33;

    // specifies a height of superscripted font
    static const sal_Int8 SUPERSCRIPTHEIGHT = 58;

    // specifies a height of subscripted font
    static const sal_Int8 SUBSCRIPTHEIGHT = 58;

    // specifies a height of normal font
    static const short NORMALHEIGHT = 100;

    /// @throws cpo::uno::RuntimeException
    VbaFontBase(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::container::XIndexAccess >& xPalette,
        const css::uno::Reference< css::beans::XPropertySet >& xPropertySet,
        Component eWhich,
        bool bFormControl = false);

    virtual ~VbaFontBase() override;// {}

    // Attributes
    virtual cpo::uno::Any getSize() override;
    virtual void setSize( const cpo::uno::Any& _size ) override;
    virtual cpo::uno::Any getColorIndex() override;
    virtual void setColorIndex( const cpo::uno::Any& _colorindex ) override;
    virtual cpo::uno::Any getBold() override;
    virtual void setBold( const cpo::uno::Any& _bold ) override;
    virtual cpo::uno::Any getUnderline() override = 0;
    virtual void setUnderline( const cpo::uno::Any& _underline ) override = 0;
    virtual cpo::uno::Any getStrikethrough() override;
    virtual void setStrikethrough( const cpo::uno::Any& _strikethrough ) override;
    virtual cpo::uno::Any getShadow() override;
    virtual void setShadow( const cpo::uno::Any& _shadow ) override;
    virtual cpo::uno::Any getItalic() override;
    virtual void setItalic( const cpo::uno::Any& _italic ) override;
    virtual cpo::uno::Any getSubscript() override;
    virtual void setSubscript( const cpo::uno::Any& _subscript ) override;
    virtual cpo::uno::Any getSuperscript() override;
    virtual void setSuperscript( const cpo::uno::Any& _superscript ) override;
    virtual cpo::uno::Any getName() override;
    virtual void setName( const cpo::uno::Any& _name ) override;
    virtual cpo::uno::Any getColor() override ;
    virtual void setColor( const cpo::uno::Any& _color ) override ;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
