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

#ifndef INCLUDED_STARMATH_INC_ELEMENTSDOCKINGWINDOW_HXX
#define INCLUDED_STARMATH_INC_ELEMENTSDOCKINGWINDOW_HXX

#include <sfx2/dockwin.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

#include "format.hxx"
#include <memory>
#include <tuple>

#include "AccessibleSmElementsControl.hxx"

class SmDocShell;
class SmNode;
class SmParser;

class SmElement
{
    std::unique_ptr<SmNode> mpNode;
    OUString                maText;
    OUString                maHelpText;

public:
    Point mBoxLocation;
    Size  mBoxSize;

    SmElement(std::unique_ptr<SmNode>&& pNode, const OUString& aText, const OUString& aHelpText);
    virtual ~SmElement();

    const std::unique_ptr<SmNode>& getNode() const;
    const OUString& getText() const { return maText; }
    const OUString& getHelpText() const { return maHelpText; }

    virtual bool isSeparator() const { return false; }
};

class SmElementSeparator final : public SmElement
{
public:
    SmElementSeparator();

    bool isSeparator() const override { return true; }
};

typedef std::pair<const char*, const char*> SmElementDescr;

class SmElementsControl : public weld::CustomWidgetController
{
    friend class ElementSelectorUIObject;
    friend class ElementUIObject;

    static const SmElementDescr m_aUnaryBinaryOperatorsList[];
    static const SmElementDescr m_aRelationsList[];
    static const SmElementDescr m_aSetOperationsList[];
    static const SmElementDescr m_aFunctionsList[];
    static const SmElementDescr m_aOperatorsList[];
    static const SmElementDescr m_aAttributesList[];
    static const SmElementDescr m_aBracketsList[];
    static const SmElementDescr m_aFormatsList[];
    static const SmElementDescr m_aOthersList[];
    static const SmElementDescr m_aExamplesList[];
    static const std::tuple<const char*, const SmElementDescr*, size_t> m_aCategories[];
    static const size_t m_aCategoriesSize;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual OUString RequestHelp(tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    SmDocShell*   mpDocShell;
    SmFormat      maFormat;
    OString       msCurrentSetId;
    sal_uInt16    m_nCurrentElement;
    sal_uInt16    m_nCurrentRolloverElement;
    sal_uInt16 m_nCurrentOffset;
    Link<SmElement&,void> maSelectHdlLink;

    std::vector< std::unique_ptr<SmElement> > maElementList;
    Size          maMaxElementDimensions;
    bool          mbVerticalMode;
    std::unique_ptr<weld::ScrolledWindow> mxScroll;
    bool m_bFirstPaintAfterLayout;
    rtl::Reference<AccessibleSmElementsControl> m_xAccessible;

    void addElement(SmParser &rParser, const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText);
    void addElements(const SmElementDescr aElementsArray[], sal_uInt16 size);
    SmElement* current() const;
    void setCurrentElement(sal_uInt16);
    bool hasRollover() const { return m_nCurrentRolloverElement != SAL_MAX_UINT16; }

    void stepFocus(const bool bBackward);
    void pageFocus(const bool bBackward);
    // common code of page and step focus
    inline void scrollToElement(const bool, const SmElement*);
    inline sal_uInt16 nextElement(const bool, const sal_uInt16, const sal_uInt16);

    void build();

    //if bDraw is true, then draw, otherwise just layout
    void LayoutOrPaintContents(vcl::RenderContext& rContext, bool bDraw);

public:
    explicit SmElementsControl(std::unique_ptr<weld::ScrolledWindow> xScrolledWindow);
    virtual ~SmElementsControl() override;

    static const auto& categories() { return m_aCategories; }
    static size_t categoriesSize() { return m_aCategoriesSize; }
    const OString& elementSetId() const { return msCurrentSetId; }
    void setElementSetId(const char* pSetId);

    void setVerticalMode(bool bVertical);

    sal_uInt16 itemCount() const;
    sal_uInt16 itemHighlighted() const;
    sal_uInt16 itemAtPos(const Point& rPos) const;
    tools::Rectangle itemPosRect(sal_uInt16) const;
    bool itemIsSeparator(sal_uInt16) const;
    bool itemIsVisible(sal_uInt16) const;
    OUString itemName(sal_uInt16) const;
    bool itemTrigger(sal_uInt16);
    void setItemHighlighted(sal_uInt16);
    sal_uInt16 itemOffset() const { return m_nCurrentOffset; }

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void );

    void SetSelectHdl(const Link<SmElement&,void>& rLink) { maSelectHdlLink = rLink; }

    const rtl::Reference<AccessibleSmElementsControl> & GetAccessible() const { return m_xAccessible; }
    static Color GetTextColor();
    static Color GetControlBackground();

    virtual FactoryFunction GetUITestFactory() const override;
};

class SmElementsDockingWindow final : public SfxDockingWindow
{
    std::unique_ptr<SmElementsControl> mxElementsControl;
    std::unique_ptr<weld::CustomWeld> mxElementsControlWin;
    std::unique_ptr<weld::ComboBox> mxElementListBox;

    virtual void Resize() override;
    SmViewShell* GetView();

    DECL_LINK(SelectClickHandler, SmElement&, void);
    DECL_LINK(ElementSelectedHandle, weld::ComboBox&, void);

public:

    SmElementsDockingWindow( SfxBindings* pBindings,
                             SfxChildWindow* pChildWindow,
                             vcl::Window* pParent );
    virtual ~SmElementsDockingWindow() override;
    virtual void dispose() override;

    virtual void EndDocking( const tools::Rectangle& rReactangle, bool bFloatMode) override;
    virtual void ToggleFloatingMode() override;
};

class SmElementsDockingWindowWrapper final : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmElementsDockingWindowWrapper);

    SmElementsDockingWindowWrapper( vcl::Window* pParentWindow,
                                    sal_uInt16 nId,
                                    SfxBindings* pBindings,
                                    SfxChildWinInfo* pInfo );
    virtual ~SmElementsDockingWindowWrapper() override;
};

#endif // INCLUDED_STARMATH_INC_ELEMENTSDOCKINGWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
