/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef OOX_DRAWINGML_TEXTFIELD_HXX
#define OOX_DRAWINGML_TEXTFIELD_HXX

#include <boost/shared_ptr.hpp>

#include "oox/drawingml/textrun.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"

namespace oox { namespace drawingml {

struct TextCharacterProperties;

class TextField
    : public TextRun
{
public:
    TextField();

    inline TextParagraphProperties& getTextParagraphProperties() { return maTextParagraphProperties; }
    inline const TextParagraphProperties& getTextParagraphProperties() const { return maTextParagraphProperties; }

    inline void setType( const ::rtl::OUString& sType ) { msType = sType; }
    inline void setUuid( const ::rtl::OUString & sUuid ) { msUuid = sUuid; }

    virtual void    insertAt(
                        const ::oox::core::XmlFilterBase& rFilterBase,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > &xAt,
                        const TextCharacterProperties& rTextCharacterStyle ) const;

private:
    TextParagraphProperties  maTextParagraphProperties;
    ::rtl::OUString msType;
    ::rtl::OUString msUuid;
};

typedef boost::shared_ptr< TextField > TextFieldPtr;

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
