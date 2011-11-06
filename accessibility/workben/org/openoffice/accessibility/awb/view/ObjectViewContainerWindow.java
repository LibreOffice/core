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

package org.openoffice.accessibility.awb.view;

import java.awt.BorderLayout;
import javax.swing.JFrame;
import com.sun.star.accessibility.XAccessibleContext;


/** Top level window that creates a single object view container.  This
    container shows information about a specific accessible object and is
    not affected by the selection of the accessbility tree widget.
*/
public class ObjectViewContainerWindow
    extends JFrame
{
    public ObjectViewContainerWindow (XAccessibleContext xContext)
    {
        setSize (new java.awt.Dimension (300,600));

        maContainer = new ObjectViewContainer ();
        maContainer.SetObject (xContext);
        getContentPane().add (maContainer, BorderLayout.CENTER);

        pack ();
        setVisible (true);
    }

    /** Set the object that is displayed in this window.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        maContainer.SetObject (xContext);
    }

    private ObjectViewContainer maContainer;
}
