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

#ifndef INCLUDED_OOX_DRAWINGML_TEXTBODY_HXX
#define INCLUDED_OOX_DRAWINGML_TEXTBODY_HXX

#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/textbodyproperties.hxx>
#include <drawingml/textliststyle.hxx>
#include <drawingml/shape3dproperties.hxx>
#include <oox/helper/refvector.hxx>

namespace com::sun::star::text {
    class XText;
    class XTextCursor;
}

namespace oox::core { class XmlFilterBase; }

namespace oox::drawingml {

class TextParagraph;
typedef RefVector< TextParagraph > TextParagraphVector;

class TextBody
{
public:
    TextBody();
    TextBody( const TextBodyPtr& pBody );

    const TextParagraphVector&          getParagraphs() const { return maParagraphs; }
    TextParagraph&                      addParagraph();
    TextParagraph&                      addParagraph(sal_Int32 nIndex);

    const TextListStyle&                getTextListStyle() const { return maTextListStyle; }
    TextListStyle&                      getTextListStyle() { return maTextListStyle; }

    const TextBodyProperties&           getTextProperties() const { return maTextProperties; }
    TextBodyProperties&                 getTextProperties() { return maTextProperties; }

    Text3DProperties&                   get3DProperties() { return ma3DProperties; }
    const Text3DProperties&             get3DProperties() const { return ma3DProperties; }

    /** insert the text body at the text cursor */
    void                insertAt(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const css::uno::Reference < css::text::XText > & xText,
                            const css::uno::Reference < css::text::XTextCursor > & xAt,
                            const TextCharacterProperties& rTextStyleProperties,
                            const TextListStylePtr& pMasterTextListStyle ) const;
    bool isEmpty() const;
    OUString toString() const;

    /** Returns whether the textbody had a rPr tag in it that alters it visually
     *
     *  For instance _lang_ doesn't have a visual effect.
     */
    bool hasVisualRunProperties() const;

    /// Returns whether the textbody had a pPr tag in it
    bool hasParagraphProperties() const;

    /// Returns whether the textbody had a non-empty bodyPr tag in it
    bool hasNoninheritedBodyProperties() const { return mbHasNoninheritedBodyProperties; }
    /// Flags textbody as having a non-empty bodyPr tag
    void setHasNoninheritedBodyProperties() { mbHasNoninheritedBodyProperties = true; }

    /// Returns whether the textbody had a non-empty lstStyle tag in it
    bool hasListStyleOnImport() const { return maTextListStyle.hasListStyleOnImport(); }

    void                ApplyStyleEmpty(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const css::uno::Reference < css::text::XText > & xText,
                            const TextCharacterProperties& rTextStyleProperties,
                            const TextListStylePtr& pMasterTextListStylePtr) const;
private:
    TextParagraphVector maParagraphs;
    TextBodyProperties  maTextProperties;
    /// Set if bodyPr tag in this textbody is non-empty during import
    bool                mbHasNoninheritedBodyProperties;
    TextListStyle       maTextListStyle;
    Text3DProperties    ma3DProperties;
};

}

#endif // INCLUDED_OOX_DRAWINGML_TEXTBODY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
