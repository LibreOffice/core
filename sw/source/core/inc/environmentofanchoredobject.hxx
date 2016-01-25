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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_ENVIRONMENTOFANCHOREDOBJECT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_ENVIRONMENTOFANCHOREDOBJECT_HXX

class SwFrame;
class SwLayoutFrame;

namespace objectpositioning
{
    class SwEnvironmentOfAnchoredObject
    {
    private:
        const bool mbFollowTextFlow;

    public:
        /** constructor

            @param _bFollowTextFlow
            input parameter - indicates, if the anchored object, for which
            this environment is instantiated, follow the text flow or not
        */
        SwEnvironmentOfAnchoredObject( const bool _bFollowTextFlow );

        /** destructor
        */
        ~SwEnvironmentOfAnchoredObject();

        /** determine environment layout frame for possible horizontal object
            positions respectively for alignment to 'page areas'

            this is, if object has to follow the text flow:
            - cell frame, if anchored inside a cell
            - fly frame, if anchored inside a fly frame
            otherwise it's the page frame

            this is, if object hasn't to follow the text flow:
            - page frame.
            - no exception any more. Thus remove
            parameter <_bForPageAlignment>

            @param _rHoriOrientFrame
            input parameter - frame, at which the horizontal position is
            oriented at (typically it's the anchor frame).
            starting point for the search of the layout frame.

            @return reference to the layout frame, which determines the
            the horizontal environment the object has to be positioned in.
        */
        const SwLayoutFrame& GetHoriEnvironmentLayoutFrame( const SwFrame& _rHoriOrientFrame ) const;

        /** determine environment layout frame for possible vertical object
            positions respectively for alignments to 'page areas'

            this is, if object has to follow the text flow:
            - cell frame, if anchored inside a cell
            - fly frame, if anchored inside a fly frame
            - header/footer frame, if anchored inside page header/footer
            - footnote frame, if anchored inside footnote
            otherwise it's the document body frame

            this is, if object hasn't to follow the text flow:
            - page frame.
            - no exception any more. Thus remove
            parameter <_bForPageAlignment>

            @param _rVertOrientFrame
            input parameter - frame, at which the vertical position is
            oriented at (typically it's the anchor frame).
            starting point for the search of the layout frame.

            @return reference to the layout frame, which determines the
            the vertical environment the object has to be positioned in.
        */
        const SwLayoutFrame& GetVertEnvironmentLayoutFrame( const SwFrame& _rVertOrientFrame ) const;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
