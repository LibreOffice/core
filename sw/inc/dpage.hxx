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
#ifndef INCLUDED_SW_INC_DPAGE_HXX
#define INCLUDED_SW_INC_DPAGE_HXX

#include <memory>
#include <svx/fmpage.hxx>
#include <svx/svdobj.hxx>

class SdrPageGridFrameList;
class SwDrawModel;
class SwDoc;
class SwXShape;
class SdrMarkList;

class SwDPage final : public FmFormPage, public SdrObjUserCall
{
    SwDPage &operator=(const SwDPage&) = delete;
    SwDPage(const SwDPage&) = delete;

    std::unique_ptr<SdrPageGridFrameList>   m_pGridLst;
    SwDoc*                                  m_pDoc;
    SdrPageView*        m_pPageView;
    std::vector<SwXShape*> m_vShapes;

public:
    explicit SwDPage(SwDrawModel& rNewModel, bool bMasterPage);
    virtual ~SwDPage() override;

    virtual rtl::Reference<SdrPage> CloneSdrPage(SdrModel& rTargetModel) const override;

    // #i3694#
    // This GetOffset() method is not needed anymore, it even leads to errors.
    // virtual Point GetOffset() const;
    virtual SdrObject* ReplaceObject( SdrObject* pNewObj, size_t nObjNum ) override;

    virtual const SdrPageGridFrameList* GetGridFrameList(const SdrPageView* pPV,
                                    const tools::Rectangle *pRect) const override;

    bool RequestHelp( vcl::Window* pWindow, SdrView const * pView, const HelpEvent& rEvt );

    const SdrMarkList&  PreGroup(const css::uno::Reference< css::drawing::XShapes >& rShapes);
    void                PreUnGroup(const css::uno::Reference< css::drawing::XShapeGroup >& rShapeGroup);

    SdrView*            GetDrawView() {return mpView.get();}
    SdrPageView*        GetPageView();
    void                RemovePageView();
    static css::uno::Reference<css::drawing::XShape> GetShape(SdrObject* pObj);
    static css::uno::Reference<css::drawing::XShapeGroup> GetShapeGroup(SdrObject* pObj);

    // The following method is called when a SvxShape-object is to be created.
    // Derived classes may obtain at this point a derivation or an object
    // that is aggregating a SvxShape.
    virtual css::uno::Reference< css::drawing::XShape >  CreateShape( SdrObject *pObj ) const override;
    void RemoveShape(const SwXShape* pShape)
    {
        auto ppShape = find(m_vShapes.begin(), m_vShapes.end(), pShape);
        if(ppShape != m_vShapes.end())
            m_vShapes.erase(ppShape);
    };

private:
    void lateInit(const SwDPage& rSrcPage);
};

#endif // INCLUDED_SW_INC_DPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
