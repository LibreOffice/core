/*************************************************************************
 *
 *  $RCSfile: simplenameclashresolverequest.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2001-05-29 11:41:47 $
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

#ifndef _UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX
#define _UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif

namespace ucbhelper {

/**
  * This class implements a simple name clash resolve interaction request.
  * Instances can be passed directly to XInteractionHandler::handle(...). Each
  * instance contains an NameClashResolveRequest and two interaction
  * continuations: "Abort" and "SupplyName". Another continuation
  * ("ReplaceExistingData") may be supplied optionally.
  *
  * @see com::sun::star::ucb::NameClashResolveRequest
  * @see InteractionAbort
  * @see InteractioneplaceExistingData
  * @see InteractionSupplyName
  */
class SimpleNameClashResolveRequest : public ucbhelper::InteractionRequest
{
    rtl::Reference< InteractionSupplyName > m_xNameSupplier;

public:
    /**
      * Constructor.
      *
      * @param rTargetFolderURL contains the URL of the folder that contains
      *        the clashing resource.
      * @param rClashingName contains the clashing name,
      * @param rProposedNewName contains a proposal for the new name or is
      *        empty.
      * @param bSupportsOverwriteData indictes whether an
      *        InteractioneplaceExistingData continuation shall be supplied
      *        with the interaction request.
      */
    SimpleNameClashResolveRequest( const rtl::OUString & rTargetFolderURL,
                                   const rtl::OUString & rClashingName,
                                   const rtl::OUString & rProposedNewName,
                                   sal_Bool bSupportsOverwriteData = sal_True );
    /**
      * This method returns the new name that was supplied by the interaction
      * handler.
      *
      * @return the new name, if supplied.
      */
    const rtl::OUString getNewName() const
    { return m_xNameSupplier->getName(); }

};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX */
