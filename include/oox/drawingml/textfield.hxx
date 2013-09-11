/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

    TextParagraphProperties& getTextParagraphProperties() { return maTextParagraphProperties; }
    const TextParagraphProperties& getTextParagraphProperties() const { return maTextParagraphProperties; }

    void setType( const OUString& sType ) { msType = sType; }
    void setUuid( const OUString & sUuid ) { msUuid = sUuid; }

    virtual sal_Int32    insertAt(
                        const ::oox::core::XmlFilterBase& rFilterBase,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > &xAt,
                        const TextCharacterProperties& rTextCharacterStyle ) const;

private:
    TextParagraphProperties  maTextParagraphProperties;
    OUString msType;
    OUString msUuid;
};

typedef boost::shared_ptr< TextField > TextFieldPtr;

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
