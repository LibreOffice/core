/*************************************************************************
 *
 *  $RCSfile: paramdialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:34:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_PARAMDIALOG_HXX_
#include "paramdialog.hxx"
#endif
#ifndef _DBAUI_PARAMDIALOG_HRC_
#include "paramdialog.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif

#define EF_VISITED      0x0001
#define EF_DIRTY        0x0002

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::util;
    using namespace ::connectivity;

    //==================================================================
    //= OParameterDialog
    //==================================================================

    //------------------------------------------------------------------------------
    #define INIT_MEMBERS()                                          \
        :ModalDialog( pParent, ModuleRes(DLG_PARAMETERS))           \
        ,m_aNamesFrame  (this, ResId(GB_PARAMS))                    \
        ,m_aAllParams   (this, ResId(LB_ALLPARAMS))                 \
        ,m_aValueFrame  (this, ResId(GB_DATA))                      \
        ,m_aParam       (this, ResId(ET_PARAM))                     \
        ,m_aTravelNext  (this, ResId(BT_TRAVELNEXT))                \
        ,m_aDelimiter   (this, ResId(FT_DELIM))                     \
        ,m_aOKBtn       (this, ResId(BT_OK))                        \
        ,m_aCancelBtn   (this, ResId(BT_CANCEL))                    \
        ,m_nCurrentlySelected(LISTBOX_ENTRY_NOTFOUND)               \
        ,m_bNeedErrorOnCurrent(sal_True)                            \
        ,m_xConnection(_rConnNFormats)                              \
        ,m_aParser(_rxORB)                                          \


    //------------------------------------------------------------------------------

    OParameterDialog::OParameterDialog(
            Window* pParent, const Reference< XIndexAccess > & rParamContainer,
            const Reference< XConnection > & _rConnNFormats, const Reference< XMultiServiceFactory >& _rxORB)
        INIT_MEMBERS()
    {
        if (_rxORB.is())
            m_xFormatter = Reference< XNumberFormatter>(_rxORB->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"))), UNO_QUERY);
        else
            DBG_ERROR("OParameterDialog::OParameterDialog: need a service factory!");

        m_pVisitedParams = (void*)new ByteVector;

        Reference< XNumberFormatsSupplier >  xNumberFormats = ::dbtools::getNumberFormats(m_xConnection, sal_True);
        if (!xNumberFormats.is())
            ::comphelper::disposeComponent(m_xFormatter);
        else if (m_xFormatter.is())
            m_xFormatter->attachNumberFormatsSupplier(xNumberFormats);
        try
        {
            DBG_ASSERT(rParamContainer->getCount(), "OParameterDialog::OParameterDialog : can't handle empty containers !");

            m_aFinalValues.realloc(rParamContainer->getCount());
            PropertyValue* pValues = m_aFinalValues.getArray();

            for (sal_Int32 i = 0, nCount = rParamContainer->getCount(); i<nCount; ++i, ++pValues)
            {
                Reference< XPropertySet >  xParamAsSet(*(Reference< XInterface > *)rParamContainer->getByIndex(i).getValue(), UNO_QUERY);
                pValues->Name = ::comphelper::getString(xParamAsSet->getPropertyValue(PROPERTY_NAME));
                m_aAllParams.InsertEntry(pValues->Name);

                if (!pValues->Value.hasValue())
                    // it won't have a value, 'cause it's default constructed. But may be later we support
                    // initializing this dialog with values
                    pValues->Value = makeAny(::rtl::OUString());
                    // default the values to an empty string

                ((ByteVector*)m_pVisitedParams)->push_back(0);
                    // not visited, not dirty
            }

            m_xParams = rParamContainer;
        }
        catch(Exception&)
        {
            DBG_ERROR("OParameterDialog::OParameterDialog : soemthing went wrong while retrieving the parameters !");
        }


        Construct();

        m_aResetVisitFlag.SetTimeoutHdl(LINK(this, OParameterDialog, OnVisitedTimeout));

        FreeResource();
    }

    //------------------------------------------------------------------------------
    OParameterDialog::~OParameterDialog()
    {
        if (m_aResetVisitFlag.IsActive())
            m_aResetVisitFlag.Stop();

        delete (ByteVector*)m_pVisitedParams;
    }

    //------------------------------------------------------------------------------
    void OParameterDialog::Construct()
    {
        m_aAllParams.SetSelectHdl(LINK(this, OParameterDialog, OnEntrySelected));
        m_aParam.SetLoseFocusHdl(LINK(this, OParameterDialog, OnValueLoseFocus));
        m_aParam.SetModifyHdl(LINK(this, OParameterDialog, OnValueModified));
        m_aTravelNext.SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));
        m_aOKBtn.SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));
        m_aCancelBtn.SetClickHdl(LINK(this, OParameterDialog, OnButtonClicked));

        if (m_aAllParams.GetEntryCount())
        {
            m_aAllParams.SelectEntryPos(0);
            LINK(this, OParameterDialog, OnEntrySelected).Call(&m_aAllParams);

            if (m_aAllParams.GetEntryCount() == 1)
            {
                m_aTravelNext.Enable(sal_False);
            }

            if (m_aAllParams.GetEntryCount() > 1)
            {
                m_aOKBtn.SetStyle(m_aOKBtn.GetStyle() & ~WB_DEFBUTTON);
                m_aTravelNext.SetStyle(m_aTravelNext.GetStyle() | WB_DEFBUTTON);
            }
        }

        m_aParam.GrabFocus();
    }

    //------------------------------------------------------------------------------
    OSQLParseNode* OParameterDialog::implPredicateTree(::rtl::OUString& _rErrorMessage, const UniString& _rStatement, const Reference< XPropertySet > & _rxField)
    {
        ::rtl::OUString aErr;
        OSQLParseNode* pReturn = m_aParser.predicateTree(aErr, _rStatement, m_xFormatter, _rxField);
        _rErrorMessage = aErr;
        if (!pReturn)
        {   // is it a text field ?
            sal_Int32 nType = DataType::OTHER;
            try { nType = ::comphelper::getINT32(_rxField->getPropertyValue(PROPERTY_TYPE)); }
            catch(Exception&) { }
            if ((DataType::CHAR == nType) || (DataType::VARCHAR == nType) || (DataType::LONGVARCHAR == nType))
            {   // yes -> force a quoted text and try again
                UniString sQuoted(_rStatement);
                if  (   sQuoted.Len()
                    &&  (   (sQuoted.GetChar(0) != '\'')
                        ||  (sQuoted.GetChar(sQuoted.Len() -1) != '\'')
                        )
                    )
                {
                    sQuoted.SearchAndReplaceAll('\'', String::CreateFromAscii("''"));
                    UniString sTemp('\'');
                    (sTemp += sQuoted) += '\'';
                    sQuoted = sTemp;
                }
                pReturn = m_aParser.predicateTree(aErr, sQuoted, m_xFormatter, _rxField);
                _rErrorMessage = aErr;
            }
        }
        return pReturn;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnValueLoseFocus, Control*, pSource)
    {
        if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
        {
            if (((*(ByteVector*)m_pVisitedParams)[m_nCurrentlySelected] & EF_DIRTY) == 0)
                // nothing to do, the value isn't dirty
                return 0L;
        }

        DBG_ASSERT(pSource == &m_aParam, "OParameterDialog::OnValueLoseFocus : invalid source !");

        // transform the current string according to the param field type
        ::rtl::OUString sTransformedText(m_aParam.GetText());
        Reference< XPropertySet >  xParamAsSet(*(Reference< XInterface > *)m_xParams->getByIndex(m_nCurrentlySelected).getValue(), UNO_QUERY);
        if (xParamAsSet.is())
        {
            if (m_xConnection.is() && m_xFormatter.is())
            {
                ::rtl::OUString sError;
                OSQLParseNode* pParseNode = implPredicateTree(sError, sTransformedText, xParamAsSet);
                if (pParseNode)
                {
                    sTransformedText = ::rtl::OUString();

                    String sLanguage, sCountry;
                    ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
                    pParseNode->parseNodeToPredicateStr(sTransformedText, m_xConnection->getMetaData(), m_xFormatter, xParamAsSet, Locale(sLanguage, sCountry, ::rtl::OUString()), '.');
                    m_aParam.SetText(sTransformedText);
                    delete pParseNode;

                    // with this the value isn't dirty anymore
                    if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
                        (*(ByteVector*)m_pVisitedParams)[m_nCurrentlySelected] &= ~EF_DIRTY;
                }
                else
                {
                    if (!m_bNeedErrorOnCurrent)
                        return 1L;

                    m_bNeedErrorOnCurrent = sal_False;  // will be reset in OnValueModified

                    ::rtl::OUString sName;
                    try
                    {
                        sName = ::comphelper::getString(xParamAsSet->getPropertyValue(PROPERTY_NAME));
                    }
                    catch(Exception&)
                    {
                    }

                    String sMessage;
                    {
                        OLocalResourceAccess aDummy(DLG_PARAMETERS, RSC_MODALDIALOG);
                        sMessage = String(ResId(STR_COULD_NOT_CONVERT_PARAM));
                    }
                    sMessage.SearchAndReplaceAll(String::CreateFromAscii("$name$"), sName.getStr());
                    ErrorBox(NULL, WB_OK, sMessage).Execute();
                    m_aParam.GrabFocus();
                    return 1L;
                }
            }
        }

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnButtonClicked, PushButton*, pButton)
    {
        if (&m_aCancelBtn == pButton)
        {
            // no interpreting of the given values anymore ....
            m_aParam.SetLoseFocusHdl(Link());   // no direct call from the control anymore ...
            m_bNeedErrorOnCurrent = sal_False;      // in case of any indirect calls -> no error message
            m_aCancelBtn.SetClickHdl(Link());
            m_aCancelBtn.Click();
        }
        else if (&m_aOKBtn == pButton)
        {
            // transfer the current values into the Any
            if (LINK(this, OParameterDialog, OnEntrySelected).Call(&m_aAllParams) != 0L)
            {   // there was an error interpreting the current text
                m_bNeedErrorOnCurrent = sal_True;
                    // we're are out of the complex web :) of direct and indirect calls to OnValueLoseFocus now,
                    // so the next time it is called we need an error message, again ....
                    // (TODO : there surely are better solutions for this ...)
                return 1L;
            }

            if (m_xParams.is())
            {
                // write the parameters
                try
                {
                    String sValue;
                    ::rtl::OUString sError;
                    International aAppInternational = Application::GetAppInternational();
                    PropertyValue* pValues = m_aFinalValues.getArray();
                    for (sal_Int32 i = 0, nCount = m_xParams->getCount(); i<nCount; ++i, ++pValues)
                    {
                        Reference< XPropertySet >  xParamAsSet(*(Reference< XInterface > *)m_xParams->getByIndex(i).getValue(), UNO_QUERY);

                        sValue = ::comphelper::getString(pValues->Value).getStr();
                        // a little problem : if the field was a text field the OnValueLoseFocus added two
                        // '-characters to the text. If we would give this to predicateTree this would add
                        // two  additional '-characters which we don't want. So check the field format.
                        // FS - 06.01.00 - 71532
                        sal_Bool bValidQuotedText = (sValue.Len() >= 2) && (sValue.GetChar(0) == '\'') && (sValue.GetChar(sValue.Len() - 1) == '\'');
                            // again : as OnValueLoseFocus always did a conversion on the value text, bValidQuotedText == sal_True
                            // implies that we have a text field, as no other field values will be formatted with
                            // the quote characters
                        if (bValidQuotedText)
                            sValue = sValue.Copy(1, sValue.Len() - 2);

                        sError = ::rtl::OUString();
                        OSQLParseNode* pParseNode = implPredicateTree(sError, sValue, xParamAsSet);
                        if (pParseNode)
                        {
                            OSQLParseNode* pNearlyPure = pParseNode->getByRule(OSQLParseNode::odbc_fct_spec);
                            if (pNearlyPure)
                            {
                                if ((pNearlyPure->count() >= 2) && (SQL_NODE_STRING == pNearlyPure->getChild(1)->getNodeType()))
                                    pValues->Value <<= ::rtl::OUString(pNearlyPure->getChild(1)->getTokenValue());
                            }
                            else if ((pParseNode->count() >= 3) && (SQL_NODE_STRING == pParseNode->getChild(2)->getNodeType()))
                                pValues->Value <<= ::rtl::OUString(pParseNode->getChild(2)->getTokenValue());

                            delete pParseNode;
                        }
                    }
                }
                catch(Exception&)
                {
                }

            }
            // to close the dialog (which is more code than a simple EndDialog)
            m_aOKBtn.SetClickHdl(Link());
            m_aOKBtn.Click();
        }
        else if (&m_aTravelNext == pButton)
        {
            sal_uInt16 nCurrent = m_aAllParams.GetSelectEntryPos();
            sal_uInt16 nCount = m_aAllParams.GetEntryCount();
            DBG_ASSERT(nCount == ((ByteVector*)m_pVisitedParams)->size(), "OParameterDialog::OnButtonClicked : inconsistent lists !");

            // search the next entry in list we haven't visited yet
            sal_uInt16 nNext = (nCurrent + 1) % nCount;
            while ((nNext != nCurrent) && ( (*((ByteVector*)m_pVisitedParams))[nNext] & EF_VISITED ))
                nNext = (nNext + 1) % nCount;

            if ( (*((ByteVector*)m_pVisitedParams))[nNext] & EF_VISITED )
                // there is no such "not visited yet" entry -> simpy take the next one
                nNext = (nCurrent + 1) % nCount;

            m_aAllParams.SelectEntryPos(nNext);
            LINK(this, OParameterDialog, OnEntrySelected).Call(&m_aAllParams);
            m_bNeedErrorOnCurrent = sal_True;
                // we're are out of the complex web :) of direct and indirect calls to OnValueLoseFocus now,
                // so the next time it is called we need an error message, again ....
                // (TODO : there surely are better solutions for this ...)
        }

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnEntrySelected, ListBox*, pList)
    {
        if (m_aResetVisitFlag.IsActive())
        {
            LINK(this, OParameterDialog, OnVisitedTimeout).Call(&m_aResetVisitFlag);
            m_aResetVisitFlag.Stop();
        }
        // save the old values
        if (m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND)
        {
            // do the transformation of the current text
            if (LINK(this, OParameterDialog, OnValueLoseFocus).Call(&m_aParam) != 0L)
            {   // there was an error interpreting the text
                m_aAllParams.SelectEntryPos(m_nCurrentlySelected);
                return 1L;
            }

            m_aFinalValues[m_nCurrentlySelected].Value <<= ::rtl::OUString(m_aParam.GetText());
        }

        // initialize the controls with the new values
        sal_uInt16 nSelected = m_aAllParams.GetSelectEntryPos();
        DBG_ASSERT(nSelected != LISTBOX_ENTRY_NOTFOUND, "OParameterDialog::OnEntrySelected : no current entry !");

        m_aParam.SetText(::comphelper::getString(m_aFinalValues[nSelected].Value));
        m_nCurrentlySelected = nSelected;

        // with this the value isn't dirty
        DBG_ASSERT(m_nCurrentlySelected < ((ByteVector*)m_pVisitedParams)->size(), "OParameterDialog::OnEntrySelected : invalid current entry !");
        (*(ByteVector*)m_pVisitedParams)[m_nCurrentlySelected] &= ~EF_DIRTY;

        m_aResetVisitFlag.SetTimeout(1000);
        m_aResetVisitFlag.Start();

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnVisitedTimeout, Timer*, pTimer)
    {
        DBG_ASSERT(m_nCurrentlySelected != LISTBOX_ENTRY_NOTFOUND, "OParameterDialog::OnVisitedTimeout : invalid call !");

        // mark the currently selected entry as visited
        DBG_ASSERT(m_nCurrentlySelected < ((ByteVector*)m_pVisitedParams)->size(), "OParameterDialog::OnVisitedTimeout : invalid entry !");
        (*(ByteVector*)m_pVisitedParams)[m_nCurrentlySelected] |= EF_VISITED;

        // was it the last "not visited yet" entry ?
        ConstByteVectorIterator aIter;
        for (   aIter = ((ByteVector*)m_pVisitedParams)->begin();
                aIter < ((ByteVector*)m_pVisitedParams)->end();
                ++aIter
            )
        {
            if (((*aIter) & EF_VISITED) == 0)
                break;
        }
        if (aIter == ((ByteVector*)m_pVisitedParams)->end())
        {   // yes, there isn't another one -> change the "default button"
            m_aTravelNext.SetStyle(m_aTravelNext.GetStyle() & ~WB_DEFBUTTON);
            m_aOKBtn.SetStyle(m_aOKBtn.GetStyle() | WB_DEFBUTTON);

            // set to focus to one of the buttons temporary (with this their "default"-state is really updated)
            Window* pOldFocus = Application::GetFocusWindow();

            // if the old focus window is the value edit do some preparations ...
            Selection aSel;
            if (pOldFocus == &m_aParam)
            {
                m_aParam.SetLoseFocusHdl(Link());
                aSel = m_aParam.GetSelection();
            }
            m_aTravelNext.GrabFocus();
            if (pOldFocus)
                pOldFocus->GrabFocus();

            // restore the settings for the value edit
            if (pOldFocus == &m_aParam)
            {
                m_aParam.SetLoseFocusHdl(LINK(this, OParameterDialog, OnValueLoseFocus));
                m_aParam.SetSelection(aSel);
            }
        }

        return 0L;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OParameterDialog, OnValueModified, Control*, pBox)
    {
        // mark the currently selected entry as dirty
        DBG_ASSERT(m_nCurrentlySelected < ((ByteVector*)m_pVisitedParams)->size(), "OParameterDialog::OnValueModified : invalid entry !");
        (*(ByteVector*)m_pVisitedParams)[m_nCurrentlySelected] |= EF_DIRTY;

        m_bNeedErrorOnCurrent = sal_True;

        return 0L;
    }


//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/27 08:07:57  fs
 *  OSQLParser interface was changed - adjustments
 *
 *  Revision 1.1  2000/10/26 18:06:51  fs
 *  initial checkin - parameter dialog
 *
 *
 *  Revision 1.0 26.10.00 11:56:41  fs
 ************************************************************************/

