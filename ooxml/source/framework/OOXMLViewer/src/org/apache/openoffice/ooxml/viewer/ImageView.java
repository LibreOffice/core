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

package org.apache.openoffice.ooxml.viewer;

import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.InputStream;

import javax.imageio.ImageIO;
import javax.swing.JComponent;
import javax.swing.JPanel;

@SuppressWarnings("serial")
public class ImageView
    extends JPanel
{
    public ImageView (final InputStream aInputStream)
    {
        BufferedImage aImage = null;
        try
        {
            aImage = ImageIO.read(aInputStream);
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        maImage = aImage;
    }




    public JComponent GetComponent()
    {
        return this;
    }




    @Override
    public void paintComponent (final Graphics aGraphics)
    {
        super.paintComponent(aGraphics);
        if (maImage != null)
        {
            aGraphics.drawImage(maImage, 10, 10, null);
        }
    }



    private final BufferedImage maImage;
}
