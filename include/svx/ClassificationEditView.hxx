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
#include <svx/weldeditview.hxx>
#include <editeng/svxenum.hxx>
#include <vcl/customweld.hxx>

namespace svx {

class ClassificationEditEngine : public EditEngine
{
public:
    ClassificationEditEngine(SfxItemPool* pItemPool);

    virtual OUString CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, boost::optional<Color>& rTxtColor, boost::optional<Color>& rFldColor) override;
};

class SVX_DLLPUBLIC ClassificationEditView : public WeldEditView
{
public:
    ClassificationEditView();
    virtual ~ClassificationEditView() override;

    virtual void makeEditEngine() override;

    void SetCharAttributes();

    void InsertField(const SvxFieldItem& rField);

    void InvertSelectionWeight();

    void SetNumType(SvxNumType eNumType);

    ClassificationEditEngine& getEditEngine()
    {
        return *static_cast<ClassificationEditEngine*>(m_xEditEngine.get());
    }

    EditView& getEditView()
    {
        return *m_xEditView.get();
    }

    void SetModifyHdl(const Link<LinkParamNone*,void>& rLink)
    {
        m_xEditEngine->SetModifyHdl(rLink);
    }
};

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONEDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
