/*************************************************************************
 *
 *  $RCSfile: submission.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 11:00:17 $
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

#ifndef __SUBMISSION_HXX
#define __SUBMISSION_HXX

#include <tools/urlobj.hxx>
#include <rtl/ustring.h>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>

#include "serialization.hxx"

namespace CSS = com::sun::star;

class CSubmissionPut;
class CSubmissionPost;
class CSubmissionGet;

class CCommandEnvironmentHelper : public cppu::WeakImplHelper1< CSS::ucb::XCommandEnvironment >
{
    friend class CSubmissionPut;
    friend class CSubmissionPost;
    friend class CSubmissionGet;

protected:
    CSS::uno::Reference< CSS::task::XInteractionHandler > m_aInteractionHandler;
    CSS::uno::Reference< CSS::ucb::XProgressHandler > m_aProgressHandler;
public:
    virtual CSS::uno::Reference< CSS::task::XInteractionHandler > SAL_CALL getInteractionHandler() throw (CSS::uno::RuntimeException)
    {
        return m_aInteractionHandler;
    }
    virtual CSS::uno::Reference< CSS::ucb::XProgressHandler > SAL_CALL getProgressHandler() throw (CSS::uno::RuntimeException)
    {
        return m_aProgressHandler;
    }
};

class CProgressHandlerHelper : public cppu::WeakImplHelper1< CSS::ucb::XProgressHandler >
{
    friend class CSubmissionPut;
    friend class CSubmissionPost;
    friend class CSubmissionGet;
protected:
    osl::Condition m_cFinished;
    osl::Mutex m_mLock;
    sal_Int32 m_count;
public:
    CProgressHandlerHelper()
        : m_count(0)
    {}
    virtual void SAL_CALL push( const com::sun::star::uno::Any& aStatus) throw(com::sun::star::uno::RuntimeException)
    {
        m_mLock.acquire();
        m_count++;
        m_mLock.release();
    }
    virtual void SAL_CALL update(const com::sun::star::uno::Any& aStatus) throw(com::sun::star::uno::RuntimeException)
    {
    }
    virtual void SAL_CALL pop() throw(com::sun::star::uno::RuntimeException)
    {
        m_mLock.acquire();
        m_count--;
        if (m_count == 0)
            m_cFinished.set();
        m_mLock.release();
    }
};

class CSubmission
{

protected:
    INetURLObject m_aURLObj;
    CSS::uno::Reference< CSS::xml::xpath::XXPathObject > m_aXPathObject;
    CSS::uno::Reference< CSS::xml::dom::XDocumentFragment > m_aFragment;
    rtl::OUString m_aEncoding;

public:
    enum SubmissionResult {
        SUCCESS,
        INVALID_METHOD,
        INVALID_URL,
        INVALID_ENCODING,
        E_TRANSMISSION,
        UNKNOWN_ERROR
    };

    CSubmission(const rtl::OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
        : m_aURLObj(aURL)
        , m_aFragment(aFragment)
    {}

    //    virtual CSS::uno::Sequence< rtl::OUString > getSupportedEncodings() = 0;
    virtual void setEncoding(const rtl::OUString& aEncoding)
    {
        m_aEncoding = aEncoding;
    }
    virtual SubmissionResult submit() = 0;

};

#endif
