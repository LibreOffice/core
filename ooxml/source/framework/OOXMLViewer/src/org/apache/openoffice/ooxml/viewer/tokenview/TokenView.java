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

package org.apache.openoffice.ooxml.viewer.tokenview;

import java.awt.Color;
import java.awt.Container;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.util.Iterator;
import java.util.Vector;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.UIManager;

/** A simple view of tokenized content.
 *
 *  Create the content by calling GetDocumentFactory() and using the returned
 *  factory to add tokenized text.
 */
@SuppressWarnings("serial")
public class TokenView<TokenType>
    extends JPanel
    implements MouseMotionListener, DocumentFactory.IRepaintTarget, ComponentListener
{
    public TokenView ()
    {
        maLines = new LineContainer<TokenType>();
        maFormatter = new Formatter<TokenType>();

        addMouseMotionListener(this);
        addComponentListener(this);
    }




    @Override
    public void paintComponent (final Graphics aGraphics)
    {
        super.paintComponent(aGraphics);

        final Graphics2D aG2 = (Graphics2D)aGraphics;
        aG2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_DEFAULT);

        final FormatState<TokenType> aState;
        synchronized(maLines)
        {
            aState = maFormatter.FormatText(aG2, maLines);
        }
        setPreferredSize(aState.GetTextBoundingSize());

        for (final Line<TokenType> aLine : aState)
        {
            PaintLineHighlight(aG2, aLine);
            PaintLineNumber(aG2, aLine);

            int nX = mnTextStart;
            for (final Run<TokenType> aRun : aLine)
            {
                final Color aRunColor;
                if (aRun == maRunUnderMouse)
                    aRunColor = maRunUnderMouseColor;
                else if (aRun == maHighlightedErrorRun)
                    aRunColor = maErrorHighlightColor;
                else
                    aRunColor = null;
                aRun.Paint(
                    (Graphics2D)aGraphics,
                    nX,
                    aLine.GetBottom(),
                    aRunColor);

                nX += aRun.GetWidth();
            }
        }

        aGraphics.setColor(maSeparatorColor);

        final int nTop = aGraphics.getClipBounds().y;
        final int nBottom = aGraphics.getClipBounds().y+aGraphics.getClipBounds().height;
        aGraphics.drawLine(
            mnBarPosition0,
            nTop,
            mnBarPosition0,
            nBottom);
        aGraphics.drawLine(
            mnBarPosition1,
            nTop,
            mnBarPosition1,
            nBottom);
    }




    /** Paint a line with a highlight.
     *  There are different kinds of highlight:
     *  - the current line
     *  - one of three groups of enclosing parent elements
     */
    private void PaintLineHighlight (
        final Graphics2D aG2,
        final Line<TokenType> aLine)
    {
        final Color aBackgroundColor;
        if (aLine == maHighlightedLine)
            aBackgroundColor = maHighlightColor;
        else
            aBackgroundColor = maBackgroundColor;

        final Color aBarColor;
        if (maLines.IsLineInGroup(aLine, maHighlightedGroup0))
            aBarColor = maGroupHighlightColor0;
        else if (maLines.IsLineInGroup(aLine, maHighlightedGroup1))
            aBarColor = maGroupHighlightColor1;
        else if (maLines.IsLineInGroup(aLine, maHighlightedGroup2))
            aBarColor = maGroupHighlightColor2;
        else
            aBarColor = maBackgroundColor;

        aG2.setColor(aBarColor);
        aG2.fillRect(
            0,
            aLine.GetTop(),
            mnLeftBarWidth,
            aLine.GetHeight());

        aG2.setColor(aBackgroundColor);
        aG2.fillRect(
            mnLeftBarWidth,
            aLine.GetTop(),
            getWidth() - mnLeftBarWidth,
            aLine.GetHeight());
    }




    private void PaintLineNumber (
        final Graphics2D aG2,
        final Line<TokenType> aLine)
    {
        final String sNumber = Integer.toString(aLine.GetStartOffset());
        final FontMetrics aMetrics = aG2.getFontMetrics();
        final int nWidth = aMetrics.stringWidth(sNumber);
        final int nHeight = aMetrics.getHeight();

        aG2.setColor(maLineNumberColor);
        aG2.setFont(maLineNumberFont);
        aG2.drawString(
            sNumber,
            mnBarPosition0+1 + mnNumberBarWidth-nWidth,
            aLine.GetBottom() - (aLine.GetHeight()-nHeight)/2 - aMetrics.getDescent());
    }




    @Override
    public void mouseDragged (final MouseEvent aEvent)
    {
    }




    @Override
    public void mouseMoved (final MouseEvent aEvent)
    {
        final Line<TokenType> aLine = maLines.GetLineForY(aEvent.getY());
        if (aLine != null)
        {
            UpdateHighlightedLine(aLine);
            final Run<TokenType> aRun = aLine.GetRunForX(aEvent.getX() - mnTextStart);
            SetRunUnderMouse(aRun);
        }
    }




    private void UpdateHighlightedLine (final Line<TokenType> aLine)
    {
        HighlightLine(aLine);

        final Iterator<Run<TokenType>> aRunIterator = aLine.iterator();
        if (aRunIterator.hasNext())
        {
            final Run<TokenType> aRun = aRunIterator.next();
            if (aRun.IsGroup())
                HighlightGroup(aRun);
            else
                HighlightGroup(aRun.GetParent());
        }
    }




    @Override
    public void RequestRepaint()
    {
        repaint();
    }




    public DocumentFactory<TokenType> GetDocumentFactory()
    {
        return new DocumentFactory<TokenType>(maLines, this);
    }




    private void HighlightLine (final Line<TokenType> aLine)
    {
        if (aLine != maHighlightedLine)
        {
            maHighlightedLine = aLine;
            repaint();
        }
    }




    private void HighlightGroup (final Run<TokenType> aRun)
    {
        if (maHighlightedGroup0 != aRun)
        {
            maHighlightedGroup0 = aRun;

            if (aRun != null)
            {
                final Run<TokenType> aGroup1 = aRun.GetParent();
                maHighlightedGroup1 = aGroup1;

                if (aGroup1 != null)
                {
                    final Run<TokenType> aGroup2 = aGroup1.GetParent();
                    maHighlightedGroup2 = aGroup2;
                }
            }
            repaint();
        }
    }




    @Override
    public void componentHidden(ComponentEvent e)
    {
    }




    @Override
    public void componentMoved (final ComponentEvent aEvent)
    {
        final Point aPoint = getMousePosition();
        if (aPoint != null)
            UpdateHighlightedLine(maLines.GetLineForY(aPoint.y));
    }




    @Override
    public void componentResized(ComponentEvent e)
    {
    }




    @Override
    public void componentShown(ComponentEvent e)
    {
    }




    public Run<TokenType> GetRun (final int nOffset)
    {
        final Line<TokenType> aLine = maLines.GetLineForOffset(nOffset);
        if (aLine != null)
            return aLine.GetRunForOffset(nOffset);
        else
            return null;
    }




    /** Return all runs that completely or partially lie in the range from
     *  start offset (including) and end offset (excluding).
     */
    public RunRange<TokenType> GetRuns (final int nStartOffset, final int nEndOffset)
    {
        final Vector<Run<TokenType>> aRuns = new Vector<>();

        for (final Line<TokenType> aLine : maLines.GetLinesForOffsets(nStartOffset, nEndOffset))
            for (final Run<TokenType> aRun : aLine.GetRunsForOffsets(nStartOffset, nEndOffset))
                aRuns.add(aRun);

        return new RunRange<TokenType>(aRuns);
    }




    public void MarkError (final Run<TokenType> aRun)
    {
        maHighlightedErrorRun = aRun;
        repaint();
    }




    public void ShowRun (final Run<TokenType> aRun)
    {
        final Container aComponent = getParent().getParent();
        if (aComponent instanceof JScrollPane)
            ((JScrollPane)aComponent).getVerticalScrollBar().setValue(
                    aRun.GetLine().GetTop());
    }




    private void SetRunUnderMouse (final Run<TokenType> aRun)
    {
        if (maRunUnderMouse != aRun)
        {
            maRunUnderMouse = aRun;
            if (maRunUnderMouse != null)
                setToolTipText(maRunUnderMouse.GetToolTipText());
            else
                setToolTipText(null);
            repaint();
        }
    }




    private final LineContainer<TokenType> maLines;
    private final Formatter<TokenType> maFormatter;
    private Line<TokenType> maHighlightedLine;
    private Run<TokenType> maHighlightedGroup0;
    private Run<TokenType> maHighlightedGroup1;
    private Run<TokenType> maHighlightedGroup2;
    private Run<TokenType> maHighlightedErrorRun;
    private Run<TokenType> maRunUnderMouse;

    private final static int mnLeftBarWidth = 10;
    private final static int mnNumberBarWidth = 30;
    private final static int mnBarPosition0 = mnLeftBarWidth;
    private final static int mnBarPosition1 = mnBarPosition0 + mnNumberBarWidth ;
    private final static int mnTextStart = mnBarPosition1 + 2;
    private final static Color maSeparatorColor = Color.GRAY;
    private final static Color maBackgroundColor = Color.WHITE;
    private final static Color maHighlightColor = new Color(0xB0E0E6); // Powder Blue
    private final static Color maGroupHighlightColor0 = new Color(0x32CD32); // Lime Green
    private final static Color maGroupHighlightColor1 = new Color(0x90EE90); // Light Green
    private final static Color maGroupHighlightColor2 = new Color(0xbbFfbb);
    private final static Color maErrorHighlightColor = new Color(0xff3020);
    private final static Color maLineNumberColor = new Color(0x808080);
    private final static Color maRunUnderMouseColor = maGroupHighlightColor2;
    private final static Font maLineNumberFont = UIManager.getDefaults().getFont("TextField.font").deriveFont(9.0f);

}
