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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TOCNTNTANCHOREDOBJECTPOSITION_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TOCNTNTANCHOREDOBJECTPOSITION_HXX
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

            virtual bool IsAnchoredToChar() const SAL_OVERRIDE;
            virtual const SwFrm* ToCharOrientFrm() const SAL_OVERRIDE;
            virtual const SwRect* ToCharRect() const SAL_OVERRIDE;
            // #i22341#
            virtual SwTwips ToCharTopOfLine() const SAL_OVERRIDE;

            // method to cast <SwAnchoredObjectPosition::GetAnchorFrm()> to
            // the needed type
            SwTxtFrm&       GetAnchorTxtFrm() const;

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
            virtual void CalcPosition() SAL_OVERRIDE;

            /** frame, at which the vertical position is oriented at
            */
            const SwLayoutFrm& GetVertPosOrientFrm() const { return *mpVertPosOrientFrm;}
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
