/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_EDITENG_QA_INC_TESTAUTOCORRDOC_HXX
#define INCLUDED_EDITENG_QA_INC_TESTAUTOCORRDOC_HXX

#include <sal/config.h>

#include <editeng/svxacorr.hxx>

class TestAutoCorrDoc : public SvxAutoCorrDoc
{
public:
    /// just like the real thing, this dummy modifies the rText parameter :(
    TestAutoCorrDoc(OUString &rText, LanguageType eLang)
        : m_rText(rText)
        , m_eLang(eLang)
    {
    }
    OUString const& getResult() const
    {
        return m_rText;
    }
private:
    OUString & m_rText;
    LanguageType m_eLang;
    virtual bool Delete( sal_Int32 nStt, sal_Int32 nEnd ) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::Delete\n");
        m_rText = m_rText.replaceAt(nStt, nEnd-nStt, "");
        return true;
    }
    virtual bool Insert( sal_Int32 nPos, const OUString& rTxt ) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::Insert\n");
        m_rText = m_rText.replaceAt(nPos, 0, rTxt);
        return true;
    }
    virtual bool Replace( sal_Int32 nPos, const OUString& rTxt ) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::Replace\n");
        return ReplaceRange( nPos, rTxt.getLength(), rTxt );
    }
    virtual bool ReplaceRange( sal_Int32 nPos, sal_Int32 nLen, const OUString& rTxt ) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::ReplaceRange %d %d %s\n", nPos, nLen, OUStringToOString(rTxt, RTL_TEXTENCODING_UTF8).getStr());
        m_rText = m_rText.replaceAt(nPos, nLen, rTxt);
        return true;
    }
    virtual bool SetAttr( sal_Int32, sal_Int32, sal_uInt16, SfxPoolItem& ) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::SetAttr\n");
        return true;
    }
    virtual bool SetINetAttr( sal_Int32, sal_Int32, const OUString& ) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::SetINetAttr\n");
        return true;
    }
    virtual OUString const* GetPrevPara(bool) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::GetPrevPara\n");
        return 0;
    }
    virtual bool ChgAutoCorrWord( sal_Int32& rSttPos,
                sal_Int32 nEndPos, SvxAutoCorrect& rACorrect,
                OUString* pPara ) SAL_OVERRIDE
    {
        //fprintf(stderr, "TestAutoCorrDoc::ChgAutoCorrWord\n");

        if (m_rText.isEmpty())
            return false;

        LanguageTag aLanguageTag( m_eLang);
        const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                m_rText, rSttPos, nEndPos, *this, aLanguageTag);
        if (pFnd && pFnd->IsTextOnly())
        {
            m_rText = m_rText.replaceAt(rSttPos, nEndPos, pFnd->GetLong());
            if( pPara )
                *pPara = "";//&pCurNode->GetString();
            return true;
        }

        return false;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
