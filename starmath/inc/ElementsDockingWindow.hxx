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

#include <boost/scoped_ptr.hpp>
#include <sfx2/dockwin.hxx>
#include <svx/dlgctrl.hxx>
#include <vcl/scrbar.hxx>

#include <document.hxx>
#include <node.hxx>

class SmElement
{
    SmNodePointer   mpNode;
    OUString        maText;
    OUString        maHelpText;
public:
    Point mBoxLocation;
    Size  mBoxSize;

    SmElement(SmNodePointer pNode, OUString aText, OUString aHelpText);
    virtual ~SmElement();

    SmNodePointer getNode();
    OUString      getText()
    {
        return maText;
    }

    OUString getHelpText()
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

    virtual bool isSeparator()
    {
        return true;
    }
};

class SmElementsControl : public Control
{
    static const sal_uInt16 aUnaryBinaryOperatorsList[][2];
    static const sal_uInt16 aRelationsList[][2];
    static const sal_uInt16 aSetOperations[][2];
    static const sal_uInt16 aFunctions[][2];
    static const sal_uInt16 aOperators[][2];
    static const sal_uInt16 aAttributes[][2];
    static const sal_uInt16 aBrackets[][2];
    static const sal_uInt16 aFormats[][2];
    static const sal_uInt16 aOthers[][2];

    Link aSelectHdlLink;

    virtual void Paint(const Rectangle&);
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void MouseMove( const MouseEvent& rMEvt );

    typedef boost::shared_ptr<SmElement>    SmElementPointer;
    typedef std::vector< SmElementPointer > SmElementList;

    SmDocShell*   mpDocShell;
    SmFormat      maFormat;
    sal_uInt16    maCurrentSetId;
    SmElement*    mpCurrentElement;

    SmElementList maElementList;
    Size          maMaxElementDimensions;
    bool          mbVerticalMode;
    boost::scoped_ptr< ScrollBar > mpScroll;

    void addElement(OUString aElementVisual, OUString aElementSource, OUString aHelpText);

    void addElements(const sal_uInt16 aElementsArray[][2], sal_uInt16 size);

    void addSeparator();

    void build();

public:
    SmElementsControl(Window *pParent, const ResId& rResId);
    virtual ~SmElementsControl();

    void setElementSetId(sal_uInt16 aSetId);

    void setVerticalMode(bool bVertical);

    void SetSelectHdl(const Link& rLink)   { aSelectHdlLink = rLink; }

    DECL_LINK( ScrollHdl, void* );
    void DoScroll(long nDelta);
};

class SmElementsDockingWindow : public SfxDockingWindow
{
    static const sal_uInt16 aCategories[];

    SmElementsControl   maElementsControl;
    ListBox             maElementListBox;

    virtual void Resize();
    SmViewShell* GetView();

    DECL_LINK(SelectClickHdl, SmElement*);
    DECL_LINK(ElementSelectedHandle, ListBox*);

public:

    SmElementsDockingWindow( SfxBindings* pBindings,
                             SfxChildWindow* pChildWindow,
                             Window* pParent );
    ~SmElementsDockingWindow();

    virtual void EndDocking( const Rectangle& rReactangle, sal_Bool bFloatMode);
    virtual void ToggleFloatingMode();
};

class SmElementsDockingWindowWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmElementsDockingWindowWrapper);

protected:
    SmElementsDockingWindowWrapper( Window* pParentWindow,
                                    sal_uInt16 nId,
                                    SfxBindings* pBindings,
                                    SfxChildWinInfo* pInfo );
    virtual ~SmElementsDockingWindowWrapper();
};

#endif // INCLUDED_STARMATH_INC_ELEMENTSDOCKINGWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
