/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textrun.hxx,v $
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


#ifndef OOX_DRAWINGML_TEXTRUN_HXX
#define OOX_DRAWINGML_TEXTRUN_HXX

#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "oox/drawingml/textcharacterproperties.hxx"

namespace oox { namespace drawingml {

class TextRun
{
public:

    TextRun();
    virtual ~TextRun();

    TextCharacterPropertiesPtr  getTextCharacterProperties(){ return maTextCharacterPropertiesPtr; };
    ::rtl::OUString & getText() { return msText; }
    const ::rtl::OUString & getText() const { return msText; }

    virtual void                insertAt(
                                    const ::oox::core::XmlFilterBase& rFilterBase,
                                    const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                                    const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > &xAt,
                                    const TextCharacterPropertiesPtr& );

    void                        setLineBreak() { mbIsLineBreak = true; }
protected:
    bool                        mbIsLineBreak;
    TextCharacterPropertiesPtr  maTextCharacterPropertiesPtr;
    ::rtl::OUString             msText;
};

typedef boost::shared_ptr< TextRun > TextRunPtr;

} }

#endif  //  OOX_DRAWINGML_TEXTRUN_HXX
