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

#ifndef INCLUDED_EDITENG_PARAGRAPHDATA_HXX
#define INCLUDED_EDITENG_PARAGRAPHDATA_HXX

#include <vector>
#include <sal/types.h>

// Only for internal use, oder some kind like hPara for the few
// functions where you need it outside ( eg. moving paragraphs... )

// Unfortunately NOT only local (formerly in outliner.hxx), but also
// used in outlobj.hxx. Moved to own header

class ParagraphData
{
    friend class Paragraph;
    friend class OutlinerParaObject;

protected:
    sal_Int16           nDepth;
    sal_Int16           mnNumberingStartValue;
    bool                mbParaIsNumberingRestart;

public:
    ParagraphData( const ParagraphData& );
    ParagraphData();

    ParagraphData& operator=( const ParagraphData& );

    // compare operator
    bool operator==(const ParagraphData& rCandidate) const;

    // data read access
    sal_Int16 getDepth() const { return nDepth; }
};

typedef ::std::vector< ParagraphData > ParagraphDataVector;

#endif // INCLUDED_EDITENG_PARAGRAPHDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
