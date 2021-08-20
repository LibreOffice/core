/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <parse5.hxx>
#include <parse6.hxx>
#include <smmod.hxx>
#include <view.hxx>
#include <document.hxx>
#include <starmathdatabase.hxx>
#include <mathml/import.hxx>

#include "com/sun/star/uno/Reference.h"
#include <com/sun/star/xforms/XModel.hpp>

using namespace ::com::sun::star::i18n;

//Definition of math keywords

SmParser6::SmParser6()
    : AbstractSmParser()
    , m_nCurError(0)
    , m_nBufferIndex(0)
    , m_nTokenIndex(0)
    , m_nRow(0)
    , m_nColOff(0)
    , m_bImportSymNames(false)
    , m_bExportSymNames(false)
    , m_nParseDepth(0)
    , m_aNumCC(LanguageTag(LANGUAGE_ENGLISH_US))
    , m_pSysCC(SM_MOD()->GetSysLocale().GetCharClassPtr())
{
}

SmParser6::~SmParser6() {}

std::unique_ptr<SmTableNode> SmParser6::Parse(const OUString& rBuffer)
{
    SmParser5 aSmParser5;
    aSmParser5.SetImportSymbolNames(this->IsImportSymbolNames());
    aSmParser5.SetExportSymbolNames(this->IsExportSymbolNames());
    return aSmParser5.Parse(rBuffer);
}

std::unique_ptr<SmNode> SmParser6::ParseExpression(const OUString& rBuffer)
{
    SmParser5 aSmParser5;
    aSmParser5.SetImportSymbolNames(this->IsImportSymbolNames());
    aSmParser5.SetExportSymbolNames(this->IsExportSymbolNames());
    return aSmParser5.ParseExpression(rBuffer);
}

SmMlElement* SmParser6::ParseML(const OUString& rBuffer)
{
    if (rBuffer.indexOf(u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>") != -1)
    {
        SmViewShell* pViewSh = SmGetActiveView();
        if (pViewSh == nullptr)
            return nullptr;
        SmDocShell* pDocSh = pViewSh->GetDoc();
        if (pDocSh == nullptr)
            return nullptr;
        css::uno::Reference<css::frame::XModel> xModel(pDocSh->GetModel());
        SmMLImportWrapper aSmMLImportWrapper(xModel);
        aSmMLImportWrapper.Import(rBuffer.subView(0));
        return aSmMLImportWrapper.getElementTree();
    }
    return nullptr;
};

SmMlElement* SmParser6::ParseExpressionML(const OUString& rBuffer)
{
    if (rBuffer.indexOf(u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>") != -1)
    {
        SmViewShell* pViewSh = SmGetActiveView();
        if (pViewSh == nullptr)
            return nullptr;
        SmDocShell* pDocSh = pViewSh->GetDoc();
        if (pDocSh == nullptr)
            return nullptr;
        css::uno::Reference<css::frame::XModel> xModel(pDocSh->GetModel());
        SmMLImportWrapper aSmMLImportWrapper(xModel);
        aSmMLImportWrapper.Import(rBuffer.subView(0));
        return aSmMLImportWrapper.getElementTree();
    }
    return nullptr;
}

const SmErrorDesc* SmParser6::NextError()
{
    if (!m_aErrDescList.empty())
        if (m_nCurError > 0)
            return &m_aErrDescList[--m_nCurError];
        else
        {
            m_nCurError = 0;
            return &m_aErrDescList[m_nCurError];
        }
    else
        return nullptr;
}

const SmErrorDesc* SmParser6::PrevError()
{
    if (!m_aErrDescList.empty())
        if (m_nCurError < static_cast<int>(m_aErrDescList.size() - 1))
            return &m_aErrDescList[++m_nCurError];
        else
        {
            m_nCurError = static_cast<int>(m_aErrDescList.size() - 1);
            return &m_aErrDescList[m_nCurError];
        }
    else
        return nullptr;
}

const SmErrorDesc* SmParser6::GetError() const
{
    if (m_aErrDescList.empty())
        return nullptr;
    return &m_aErrDescList.front();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
