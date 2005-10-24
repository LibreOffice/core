/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: activedatasink.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:37:41 $
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

#ifndef _UCBHELPER_ACTIVEDATASINK_HXX
#define _UCBHELPER_ACTIVEDATASINK_HXX

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif
namespace ucb
{

//=========================================================================

/**
  * This class implements the interface com::sun::star::io::XActiveDataSink.
  * Instances of this class can be passed with the parameters of an
  * "open" command.
  */

class UCBHELPER_DLLPUBLIC ActiveDataSink : public cppu::OWeakObject,
                       public com::sun::star::lang::XTypeProvider,
                       public com::sun::star::io::XActiveDataSink
{
    com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > m_xStream;

public:
     // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XActiveDataSink methods.
    virtual void SAL_CALL
    setInputStream( const com::sun::star::uno::Reference<
                            com::sun::star::io::XInputStream >& aStream )
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > SAL_CALL
    getInputStream()
        throw( com::sun::star::uno::RuntimeException );
};

} /* namespace ucb */

#endif /* !_UCBHELPER_ACTIVEDATASINK_HXX */
