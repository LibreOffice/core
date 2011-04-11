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
#ifndef _ENVIRONMENTOFANCHOREDOBJECT_HXX
#define _ENVIRONMENTOFANCHOREDOBJECT_HXX

class SwFrm;
class SwLayoutFrm;

namespace objectpositioning
{
    class SwEnvironmentOfAnchoredObject
    {
        private:
            const bool mbFollowTextFlow;

        public:
            /** construtor

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

                @param _rHoriOrientFrm
                input parameter - frame, at which the horizontal position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @return reference to the layout frame, which determines the
                the horizontal environment the object has to be positioned in.
            */
            const SwLayoutFrm& GetHoriEnvironmentLayoutFrm( const SwFrm& _rHoriOrientFrm ) const;

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

                @param _rVertOrientFrm
                input parameter - frame, at which the vertical position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @return reference to the layout frame, which determines the
                the vertical environment the object has to be positioned in.
            */
            const SwLayoutFrm& GetVertEnvironmentLayoutFrm( const SwFrm& _rVertOrientFrm ) const;
    };
} // namespace objectpositioning

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
