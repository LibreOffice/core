/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: submission_put.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __SUBMISSION_PUT_HXX
#define __SUBMISSION_PUT_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include "submission.hxx"

class CSubmissionPut : public CSubmission
{
private:
    CSS::uno::Reference< CSS::lang::XMultiServiceFactory > m_aFactory;
public:
    CSubmissionPut(const rtl::OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment);
    virtual SubmissionResult submit(const CSS::uno::Reference< CSS::task::XInteractionHandler >& aInteractionHandler);

};
#endif

