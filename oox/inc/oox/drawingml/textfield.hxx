/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
