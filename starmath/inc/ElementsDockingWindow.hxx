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
#include <svx/dlgctrl.hxx>
#include <vcl/scrbar.hxx>

#include "format.hxx"
#include <memory>

class SmDocShell;
class SmNode;

class SmElement
{
    std::unique_ptr<SmNode> mpNode;
    OUString        maText;
    OUString        maHelpText;
public:
    Point mBoxLocation;
    Size  mBoxSize;

    SmElement(std::unique_ptr<SmNode>&& pNode, const OUString& aText, const OUString& aHelpText);
    virtual ~SmElement();

    const std::unique_ptr<SmNode>& getNode();
    const OUString& getText()
    {
        return maText;
    }

    const OUString& getHelpText()
    {
        return maHelpText;
    }

    virtual bool isSeparator()
    {
        return false;
    }
};

class SmElementSeparator : public SmElement
{
public:
    SmElementSeparator();

    virtual bool isSeparator() override
    {
        return true;
    }
};

class SmElementsControl : public Control
{
    friend class ElementSelectorUIObject;
    friend class ElementUIObject;

    static const std::pair<const char*, const char*> aUnaryBinaryOperatorsList[];
    static const std::pair<const char*, const char*> aRelationsList[];
    static const std::pair<const char*, const char*> aSetOperations[];
    static const std::pair<const char*, const char*> aFunctions[];
    static const std::pair<const char*, const char*> aOperators[];
    static const std::pair<const char*, const char*> aAttributes[];
    static const std::pair<const char*, const char*> aBrackets[];
    static const std::pair<const char*, const char*> aFormats[];
    static const std::pair<const char*, const char*> aOthers[];

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void MouseMove(const MouseEvent& rMEvt) override;
    virtual void RequestHelp(const HelpEvent& rHEvt) override;

    SmDocShell*   mpDocShell;
    SmFormat      maFormat;
    OString       msCurrentSetId;
    SmElement*    mpCurrentElement;
    Link<SmElement&,void> maSelectHdlLink;

    std::vector< std::unique_ptr<SmElement> > maElementList;
    Size          maMaxElementDimensions;
    bool          mbVerticalMode;
    VclPtr< ScrollBar > mxScroll;

    void addElement(const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText);

    void addElements(const std::pair<const char*, const char*> aElementsArray[], sal_uInt16 size);

    void build();

    //if pContext is not NULL, then draw, otherwise
    //just layout
    void LayoutOrPaintContents(vcl::RenderContext *pContext = nullptr);

public:
    explicit SmElementsControl(vcl::Window *pParent);
    virtual ~SmElementsControl() override;
    virtual void dispose() override;

    void setElementSetId(const char* pSetId);

    void setVerticalMode(bool bVertical);

    Size GetOptimalSize() const override;

    DECL_LINK( ScrollHdl, ScrollBar*, void );
    void DoScroll(long nDelta);

    void SetSelectHdl(const Link<SmElement&,void>& rLink) { maSelectHdlLink = rLink; }

    virtual FactoryFunction GetUITestFactory() const override;
};

class SmElementsDockingWindow : public SfxDockingWindow
{
    static const char* aCategories[];

    VclPtr<SmElementsControl>  mpElementsControl;
    VclPtr<ListBox>            mpElementListBox;

    virtual void Resize() override;
    SmViewShell* GetView();

    DECL_LINK(SelectClickHandler, SmElement&, void);
    DECL_LINK(ElementSelectedHandle, ListBox&, void);

public:

    SmElementsDockingWindow( SfxBindings* pBindings,
                             SfxChildWindow* pChildWindow,
                             vcl::Window* pParent );
    virtual ~SmElementsDockingWindow() override;
    virtual void dispose() override;

    virtual void EndDocking( const tools::Rectangle& rReactangle, bool bFloatMode) override;
    virtual void ToggleFloatingMode() override;
};

class SmElementsDockingWindowWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmElementsDockingWindowWrapper);

protected:
    SmElementsDockingWindowWrapper( vcl::Window* pParentWindow,
                                    sal_uInt16 nId,
                                    SfxBindings* pBindings,
                                    SfxChildWinInfo* pInfo );
    virtual ~SmElementsDockingWindowWrapper() override;
};

#endif // INCLUDED_STARMATH_INC_ELEMENTSDOCKINGWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
