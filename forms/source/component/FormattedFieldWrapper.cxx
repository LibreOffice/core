/*************************************************************************
 *
 *  $RCSfile: FormattedFieldWrapper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-19 11:52:16 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#ifndef _FRM_FORMATTED_FIELD_WRAPPER_HXX_
#include "FormattedFieldWrapper.hxx"
#endif
#ifndef _FORMS_EDIT_HXX_
#include "Edit.hxx"
#endif
#ifndef _FORMS_FORMATTEDFIELD_HXX_
#include "FormattedField.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif

//.........................................................................
namespace frm
{

//==================================================================
// OFormattedFieldWrapper
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormattedFieldWrapper(_rxFactory, sal_True));
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormattedFieldWrapper(_rxFactory, sal_False));
}

//------------------------------------------------------------------
OFormattedFieldWrapper::OFormattedFieldWrapper(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory, sal_Bool _bActAsFormatted)
    :m_pEditPart(NULL)
    ,m_xServiceFactory(_rxFactory)
{
    if (_bActAsFormatted)
    {
        increment(m_refCount);
        {
            // instantiate an FormattedModel
            InterfaceRef  xFormattedModel;
            // (instantiate it directly ..., as the OFormattedModel isn't registered for any service names anymore)
            OFormattedModel* pModel = new OFormattedModel(m_xServiceFactory);
            query_interface(static_cast<staruno::XWeak*>(pModel), xFormattedModel);

            m_xAggregate = staruno::Reference<staruno::XAggregation> (xFormattedModel, staruno::UNO_QUERY);
            DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::OFormattedFieldWrapper : the OFormattedModel didn't have an staruno::XAggregation interface !");

            // _before_ setting the delegator, give it to the member references
            query_interface(xFormattedModel, m_xFormattedPart);
            m_pEditPart = new OEditModel(m_xServiceFactory);
            m_pEditPart->acquire();
        }
        if (m_xAggregate.is())
        {   // has to be in it's own block because of the temporary variable created by *this
            m_xAggregate->setDelegator(static_cast<staruno::XWeak*>(this));
        }
        decrement(m_refCount);
    }
}

//------------------------------------------------------------------
OFormattedFieldWrapper::~OFormattedFieldWrapper()
{
    // release the aggregated object (if any)
    if (m_xAggregate.is())
        m_xAggregate->setDelegator(InterfaceRef ());

    if (m_pEditPart)
        m_pEditPart->release();
}

//------------------------------------------------------------------
staruno::Any SAL_CALL OFormattedFieldWrapper::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    if (_rType.equals( ::getCppuType( static_cast< staruno::Reference< starlang::XTypeProvider >* >(NULL) ) ) )
    {   // a XTypeProvider interface needs a working aggregate - we don't want to give the type provider
        // of our base class (OWeakAggObject) to the caller as it supplies nearly nothing
        ensureAggregate();
        if (m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    if (!aReturn.hasValue())
        aReturn = OWeakAggObject::queryAggregation(_rType);

    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<stario::XPersistObject*>(this)
        );

    if (!aReturn.hasValue())
    {
        // somebody requests an interface other than the basics (staruno::XInterface) and other than
        // the only one we can supply without an aggregate. So ensure the aggregate exists.
        ensureAggregate();
        if (m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    return aReturn;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormattedFieldWrapper::getServiceName() throw(staruno::RuntimeException)
{
    // return the old compatibility name for an EditModel
    return FRM_COMPONENT_EDIT;
}

//------------------------------------------------------------------
void SAL_CALL OFormattedFieldWrapper::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw( stario::IOException, staruno::RuntimeException )
{
    // can't write myself
    ensureAggregate();

    // if we act as real edit field, we can simple forward this write request
    if (!m_xFormattedPart.is())
    {
        staruno::Reference<stario::XPersistObject>  xAggregatePersistence;
        query_aggregation(m_xAggregate, xAggregatePersistence);
        DBG_ASSERT(xAggregatePersistence.is(), "OFormattedFieldWrapper::write : don't know how to handle this : can't write !");
            // oops ... We gave an stario::XPersistObject interface to the caller but now we aren't an stario::XPersistObject ...
        if (xAggregatePersistence.is())
            xAggregatePersistence->write(_rxOutStream);
        return;
    }

    // else we have to write an edit part first
    DBG_ASSERT(m_pEditPart, "OFormattedFieldWrapper::write : formatted part without edit part ?");

    // for this we transfer the current props of the formatted part to the edit part
    staruno::Reference<starbeans::XPropertySet>  xFormatProps(m_xFormattedPart, staruno::UNO_QUERY);
    staruno::Reference<starbeans::XPropertySet>  xEditProps;
    query_interface(static_cast<staruno::XWeak*>(m_pEditPart), xEditProps);

    UniString sLanguage, sCountry;
    ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
    starlang::Locale aAppLanguage(
        sLanguage,
        sCountry,
        ::rtl::OUString());

    dbtools::TransferFormComponentProperties(xFormatProps, xEditProps, aAppLanguage);

    // then write the edit part, after switching to "fake mode"
    m_pEditPart->enableFormattedWriteFake();
    m_pEditPart->write(_rxOutStream);
    m_pEditPart->disableFormattedWriteFake();

    // and finally write the formatted part we're really interested in
    m_xFormattedPart->write(_rxOutStream);
}

//------------------------------------------------------------------
void SAL_CALL OFormattedFieldWrapper::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw( stario::IOException, staruno::RuntimeException )
{
    if (m_xAggregate.is())
    {   //  we alread did a decision if we're an EditModel or a FormattedModel

        // if we act as formatted, we have to read the edit part first
        if (m_xFormattedPart.is())
        {
            // two possible cases:
            // a) the stuff was written by a version which didn't work with an Edit header (all intermediate
            //      versions >5.1 && <=568)
            // b) it was written by a version using edit headers
            // as we can distinguish a) from b) only after we have read the edit part, we need to remember the
            // position
            staruno::Reference<stario::XMarkableStream>  xInMarkable(_rxInStream, staruno::UNO_QUERY);
            DBG_ASSERT(xInMarkable.is(), "OFormattedFieldWrapper::read : can only work with markable streams !");
            sal_Int32 nBeforeEditPart = xInMarkable->createMark();

            m_pEditPart->read(_rxInStream);
                // this only works because an edit model can read the stuff written by a formatted model (maybe with
                // some assertions) , but not vice versa
            if (!m_pEditPart->lastReadWasFormattedFake())
            {   // case a), written with a version without the edit part fake, so seek to the start position, again
                xInMarkable->jumpToMark(nBeforeEditPart);
            }
            xInMarkable->deleteMark(nBeforeEditPart);
        }

        staruno::Reference<stario::XPersistObject>  xAggregatePersistence;
        query_aggregation(m_xAggregate, xAggregatePersistence);
        DBG_ASSERT(xAggregatePersistence.is(), "OFormattedFieldWrapper::read : don't know how to handle this : can't read !");
            // oops ... We gave an stario::XPersistObject interface to the caller but now we aren't an stario::XPersistObject ...

        if (xAggregatePersistence.is())
            xAggregatePersistence->read(_rxInStream);
        return;
    }

    // we have to decide from the data within the stream whether we should be an EditModel or a FormattedModel
    OEditBaseModel* pNewAggregate = NULL;

    // let an OEditModel do the reading
    OEditModel* pBasicReader = new OEditModel(m_xServiceFactory);
    pBasicReader->read(_rxInStream);

    // was it really an edit model ?
    if (!pBasicReader->lastReadWasFormattedFake())
        // yes -> all fine
        pNewAggregate = pBasicReader;
    else
    {   // no -> substitute it with a formatted model

        // let the formmatted model do the reading
        OFormattedModel* pFormattedReader = new OFormattedModel(m_xServiceFactory);
        pFormattedReader->read(_rxInStream);

        // for the next write (if any) : the FormattedModel and the EditModel parts
        query_interface(static_cast<staruno::XWeak*>(pFormattedReader), m_xFormattedPart);
        m_pEditPart = pBasicReader;
        m_pEditPart->acquire();

        // aggregate the formatted part below
        pNewAggregate = pFormattedReader;
    }

    // do the aggregation
    increment(m_refCount);
    {
        query_interface(static_cast<staruno::XWeak*>(pNewAggregate), m_xAggregate);
        DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::read : the OEditModel didn't have an staruno::XAggregation interface !");
    }
    if (m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        m_xAggregate->setDelegator(static_cast<staruno::XWeak*>(this));
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------
void OFormattedFieldWrapper::ensureAggregate()
{
    if (m_xAggregate.is())
        return;

    increment(m_refCount);
    {
        // instantiate an EditModel (the only place where we are allowed to decide that we're an FormattedModel
        // is in ::read)
        InterfaceRef  xEditModel = m_xServiceFactory->createInstance(FRM_SUN_COMPONENT_TEXTFIELD);
        if (!xEditModel.is())
        {
            // arghhh ... instantiate it directly ... it's dirty, but we really need this aggregate
            OEditModel* pModel = new OEditModel(m_xServiceFactory);
            query_interface(static_cast<staruno::XWeak*>(pModel), xEditModel);
        }

        m_xAggregate = staruno::Reference<staruno::XAggregation> (xEditModel, staruno::UNO_QUERY);
        DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::ensureAggregate : the OEditModel didn't have an staruno::XAggregation interface !");
    }
    if (m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        m_xAggregate->setDelegator(static_cast<staruno::XWeak*>(this));
    }
    decrement(m_refCount);
}

//.........................................................................
}
//.........................................................................


