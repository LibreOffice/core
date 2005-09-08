/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textattributelistener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:11:46 $
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
#ifndef FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX
#define FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX

#ifndef FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX
#include "rtattributes.hxx"
#endif

struct ESelection;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= ITextAttributeListener
    //====================================================================
    class ITextAttributeListener
    {
    public:
        virtual void    onAttributeStateChanged( AttributeId _nAttributeId, const AttributeState& _rState ) = 0;
    };

    //====================================================================
    //= ITextAttributeListener
    //====================================================================
    class ITextSelectionListener
    {
    public:
        virtual void    onSelectionChanged( const ESelection& _rSelection ) = 0;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX

