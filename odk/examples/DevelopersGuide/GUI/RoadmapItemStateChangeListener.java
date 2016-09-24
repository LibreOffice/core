/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.awt.XItemListener;
import com.sun.star.lang.EventObject;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;


public class RoadmapItemStateChangeListener implements XItemListener {
    private final com.sun.star.lang.XMultiServiceFactory m_xMSFDialogModel;

    public RoadmapItemStateChangeListener(com.sun.star.lang.XMultiServiceFactory xMSFDialogModel) {
        m_xMSFDialogModel = xMSFDialogModel;
    }

    public void itemStateChanged(com.sun.star.awt.ItemEvent itemEvent) {
        try {
            // get the new ID of the roadmap that is supposed to refer to the new step of the dialogmodel
            int nNewID = itemEvent.ItemId;
            XPropertySet xDialogModelPropertySet = UnoRuntime.queryInterface(XPropertySet.class, m_xMSFDialogModel);
            int nOldStep = ((Integer) xDialogModelPropertySet.getPropertyValue("Step")).intValue();
            // in the following line "ID" and "Step" are mixed together.
            // In fact in this case they denot the same
            if (nNewID != nOldStep){
                xDialogModelPropertySet.setPropertyValue("Step", Integer.valueOf(nNewID));
            }
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
        }
    }

    public void disposing(EventObject eventObject) {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
