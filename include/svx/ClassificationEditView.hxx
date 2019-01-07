/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_CLASSIFICATIONEDITVIEW_HXX
#define INCLUDED_SVX_CLASSIFICATIONEDITVIEW_HXX

#include <sal/config.h>
#include <svx/svxdllapi.h>
#include <vcl/ctrl.hxx>
#include <editeng/flditem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

namespace svx {

class ClassificationEditEngine : public EditEngine
{
public:
    ClassificationEditEngine(SfxItemPool* pItemPool);

    virtual OUString CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, boost::optional<Color>& rTxtColor, boost::optional<Color>& rFldColor) override;
};

class SVX_DLLPUBLIC ClassificationEditView : public Control
{
public:
    ClassificationEditView(vcl::Window* pParent,  WinBits nBits);
    virtual ~ClassificationEditView() override;

    using Control::SetFont;
    using Control::SetText;

    void SetCharAttributes();

    void InsertField(const SvxFieldItem& rField);

    void InvertSelectionWeight();

    void SetNumType(SvxNumType eNumType);

    std::unique_ptr<ClassificationEditEngine> pEdEngine;
    std::unique_ptr<EditView> pEdView;

    const ClassificationEditEngine& getEditEngine()
    {
        return *pEdEngine;
    }

    void SetModifyHdl(const Link<LinkParamNone*,void>& rLink)
    {
        pEdEngine->SetModifyHdl(rLink);
    }

protected:
    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void Command( const CommandEvent& rCEvt ) override;
    virtual void GetFocus() override;
    virtual void Resize() override;
};

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONEDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
