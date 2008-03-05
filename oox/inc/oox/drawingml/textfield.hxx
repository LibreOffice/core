/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textfield.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:44:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef OOX_DRAWINGML_TEXTFIELD_HXX
#define OOX_DRAWINGML_TEXTFIELD_HXX

#include <boost/shared_ptr.hpp>

#include "oox/drawingml/textrun.hxx"
#include "oox/drawingml/textcharacterproperties.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"

namespace oox { namespace drawingml {

    class TextField
        : public TextRun
    {
    public:
        TextField();

        TextParagraphPropertiesPtr getTextParagraphProperties()
            { return mpTextParagraphPropertiesPtr; }

        virtual void    insertAt(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > &xAt,
                            const TextCharacterPropertiesPtr& );

        void setType(const ::rtl::OUString & sType)
            { msType = sType; }
        void setUuid(const ::rtl::OUString & sUuid)
            { msUuid = sUuid; }
    private:
        TextParagraphPropertiesPtr  mpTextParagraphPropertiesPtr;
        ::rtl::OUString msType;
        ::rtl::OUString msUuid;
    };

    typedef boost::shared_ptr< TextField > TextFieldPtr;
} }

#endif
