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

#ifndef LAYOUTPARSE_HXX
#define LAYOUTPARSE_HXX

#include "xmlparse.hxx"

class LayoutXMLFile : public XMLFile
{
    bool mMergeMode;

public:
    LayoutXMLFile( bool mergeMode );

    void SearchL10NElements( XMLParentNode *pCur, int pos = 0 );
    sal_Bool Write( ByteString &aFilename );
    void HandleElement( XMLElement* element );
    void InsertL10NElement( ByteString const& id, XMLElement* element );

    using XMLFile::InsertL10NElement;
    using XMLFile::Write;
};

std::vector<XMLAttribute*> interestingAttributes( XMLAttributeList* lst );

#endif /* LAYOUTPARSE_HXX */
