/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TransformerActionInit.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:57:15 $
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

#ifndef _XMLOFF_TRANSFORMERACTIONINIT_HXX
#define _XMLOFF_TRANSFORMERACTIONINIT_HXX

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif


struct XMLTransformerActionInit
{
    sal_uInt16 m_nPrefix;
    ::xmloff::token::XMLTokenEnum m_eLocalName;
    sal_uInt32 m_nActionType;
    sal_uInt32 m_nParam1;
    sal_uInt32 m_nParam2;
    sal_uInt32 m_nParam3;

    static sal_Int32 QNameParam( sal_uInt16 nPrefix,
                                    ::xmloff::token::XMLTokenEnum eLocalName )
    {
        return (static_cast< sal_uInt32 >( nPrefix ) << 16) +
               static_cast< sal_uInt32 >( eLocalName );
    }
};

#endif  //  _XMLOFF_TRANSFORMERACTIONINIT_HXX
