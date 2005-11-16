/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: checkit.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 09:28:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CHECKIT_HXX
#define _CHECKIT_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_I18N_XEXTENDEDINPUTSEQUENCECHECKER_HDL_
#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>
#endif

/*************************************************************************
 * Input strings with length > MAX_SEQUENCE_CHECK_LEN are not checked.
 *************************************************************************/

#define MAX_SEQUENCE_CHECK_LEN 5

/*************************************************************************
 *                      class SwCheckIt
 *
 * Wrapper for the XInputSequenceChecker
 *************************************************************************/

class SwCheckIt
{
public:
    com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedInputSequenceChecker > xCheck;

    SwCheckIt();
};

extern SwCheckIt* pCheckIt;

#endif

