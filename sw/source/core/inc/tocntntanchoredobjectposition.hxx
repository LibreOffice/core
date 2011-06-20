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
#ifndef _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#define _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>

class SwFrm;
class SwTxtFrm;
class SwLayoutFrm;
class SwRect;

namespace objectpositioning
{
    class SwToCntntAnchoredObjectPosition : public SwAnchoredObjectPosition
    {
        private:
            // calculated data for object position
            const SwLayoutFrm* mpVertPosOrientFrm;
            // #i26791#
            // determine offset to frame anchor position according to the
            // positioning alignments
            Point maOffsetToFrmAnchorPos;

            // data for calculation of position
            bool          mbAnchorToChar;
            const SwFrm*  mpToCharOrientFrm;
            const SwRect* mpToCharRect;
            SwTwips       mnToCharTopOfLine;

            virtual bool IsAnchoredToChar() const;
            virtual const SwFrm* ToCharOrientFrm() const;
            virtual const SwRect* ToCharRect() const;
            // #i22341#
            virtual SwTwips ToCharTopOfLine() const;

            // method to cast <SwAnchoredObjectPosition::GetAnchorFrm()> to
            // the needed type
            SwTxtFrm&       GetAnchorTxtFrm() const;

        // *********************************************************************
            /** determine frame for horizontal position

                if the given proposed frame is a content frame, the proposed
                frame is returned.
                otherwise (given proposed frame is a layout frame),
                the lower content frames of the proposed frame are checked
                for the first, that the anchor or a follow of the anchor.
                If none is found, the proposed frame is returned.

                @param _pProposedFrm
                input parameter - proposed frame for horizontal position

                @return constant reference to <SwFrm> object, at which the
                horizontal position is determined.
            */
            const SwFrm& _GetHoriVirtualAnchor( const SwLayoutFrm& _pProposedFrm ) const;

        public:
            SwToCntntAnchoredObjectPosition( SdrObject& _rDrawObj );
            virtual ~SwToCntntAnchoredObjectPosition();

            /** calculate position of object
            */
            virtual void CalcPosition();

            /** frame, at which the vertical position is oriented at
            */
            const SwLayoutFrm& GetVertPosOrientFrm() const;
    };
} // namespace objectpositioning

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
