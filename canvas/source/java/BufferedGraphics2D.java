/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BufferedGraphics2D.java,v $
 * $Revision: 1.4 $
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

// Java AWT
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.awt.image.renderable.*;
import java.awt.font.*;
import java.text.*;
import java.util.*;


public class BufferedGraphics2D
    extends java.awt.Graphics2D
{
    // TODO: Somehow, get rid of this duplicate graphics (the graphics member,
    // and this object itself, extending a Graphics2D)
    private Graphics2D          graphics;
    private BufferedImage       backBuffer;
    private Graphics2D          backBufferGraphics;

    //----------------------------------------------------------------------------------

    public BufferedGraphics2D( java.awt.Graphics2D _graphics, int width, int height )
    {
        setGraphics( _graphics, Math.max(1,width), Math.max(1,height) );
    }

    public void redraw( Graphics2D graph )
    {
        if( graph != null &&
            backBuffer != null )
        {
            CanvasUtils.printLog("BufferedGraphics2D.redraw: using buffer of size (" +
                                 backBuffer.getWidth() + "," + backBuffer.getHeight() + ")" );

            // set transform to identity
            graph.setTransform( new AffineTransform() );
            graph.drawImage(backBuffer, 0, 0, null);
            CanvasUtils.postRenderImageTreatment( backBuffer );
        }
    }

    public BufferedImage getBackBuffer()
    {
        return backBuffer;
    }

    public void setSize( int width, int height )
    {
        if( backBuffer != null &&
            width == backBuffer.getWidth() &&
            height == backBuffer.getHeight() )
        {
            return;
        }

        if( backBufferGraphics != null )
            backBufferGraphics.dispose();

        if( backBuffer != null )
            backBuffer.flush();

        // TODO: Maybe VolatileImage with another BufferedImage as a backup is
        // a tad faster here.
        backBuffer = graphics.getDeviceConfiguration().createCompatibleImage(width,
                                                                             height);

        backBufferGraphics = backBuffer.createGraphics();
        CanvasUtils.initGraphics( backBufferGraphics );

        // clear the buffer to white (to have a defined state here)
        backBufferGraphics.setColor( java.awt.Color.white );
        backBufferGraphics.fillRect( 0,0,width,height );
    }

    public void setGraphics( Graphics2D _graphics, int width, int height )
    {
        if( graphics != null )
            graphics.dispose();

        graphics = _graphics;

        setSize(width,height);
    }

    //----------------------------------------------------------------------------------

    //
    // Graphics
    // ========
    //
    public void clearRect(int x, int y,  int width, int height)
    {
        graphics.clearRect(x,y,width,height);
        backBufferGraphics.clearRect(x,y,width,height);
    }

    public void clipRect(int x, int y, int width, int height)
    {
        graphics.clipRect(x,y,width,height);
        backBufferGraphics.clipRect(x,y,width,height);
    }

    public void copyArea(int x, int y, int width, int height, int dx, int dy)
    {
        graphics.copyArea(x,y,width,height,dx,dy);
        backBufferGraphics.copyArea(x,y,width,height,dx,dy);
    }

    public Graphics create()
    {
        return null;
    }

    public Graphics create(int x, int y, int width, int height)
    {
        return null;
    }

    public void dispose()
    {
        graphics.dispose();
        backBufferGraphics.dispose();
        backBuffer.flush();
    }

    public void draw3DRect(int x, int y, int width, int height, boolean raised)
    {
        graphics.draw3DRect(x,y,width,height,raised);
        backBufferGraphics.draw3DRect(x,y,width,height,raised);
    }

    public void drawArc(int x, int y, int width, int height, int startAngle, int arcAngle)
    {
        graphics.drawArc(x,y,width,height,startAngle,arcAngle);
        backBufferGraphics.drawArc(x,y,width,height,startAngle,arcAngle);
    }

    public void drawBytes(byte[] data, int offset, int length, int x, int y)
    {
        graphics.drawBytes(data,offset,length,x,y);
        backBufferGraphics.drawBytes(data,offset,length,x,y);
    }

    public void drawChars(char[] data, int offset, int length, int x, int y)
    {
        graphics.drawChars(data,offset,length,x,y);
        backBufferGraphics.drawChars(data,offset,length,x,y);
    }

    public boolean drawImage(Image img, int x, int y, Color bgcolor, ImageObserver observer)
    {
        backBufferGraphics.drawImage(img,x,y,bgcolor,observer);
        return graphics.drawImage(img,x,y,bgcolor,observer);
    }

    public boolean drawImage(Image img, int x, int y, ImageObserver observer)
    {
        backBufferGraphics.drawImage(img,x,y,observer);
        return graphics.drawImage(img,x,y,observer);
    }

    public boolean drawImage(Image img, int x, int y, int width, int height, Color bgcolor,  ImageObserver observer)
    {
        backBufferGraphics.drawImage(img,x,y,width,height,bgcolor,observer);
        return graphics.drawImage(img,x,y,width,height,bgcolor,observer);
    }

    public boolean drawImage(Image img, int x, int y, int width, int height, ImageObserver observer)
    {
        backBufferGraphics.drawImage(img,x,y,width,height,observer);
        return graphics.drawImage(img,x,y,width,height,observer);
    }

    public boolean drawImage(Image img, int dx1, int dy1, int dx2, int dy2, int sx1, int sy1, int sx2, int sy2, Color bgcolor, ImageObserver observer)
    {
        backBufferGraphics.drawImage(img,dx1,dy1,dx2,dy2,sx1,sy1,sx2,sy2,bgcolor,observer);
        return graphics.drawImage(img,dx1,dy1,dx2,dy2,sx1,sy1,sx2,sy2,bgcolor,observer);
    }

    public boolean drawImage(Image img, int dx1, int dy1, int dx2, int dy2, int sx1, int sy1, int sx2, int sy2, ImageObserver observer)
    {
        backBufferGraphics.drawImage(img,dx1,dy1,dx2,dy2,sx1,sy1,sx2,sy2,observer);
        return graphics.drawImage(img,dx1,dy1,dx2,dy2,sx1,sy1,sx2,sy2,observer);
    }

    public void drawLine(int x1, int y1, int x2, int y2)
    {
        graphics.drawLine(x1,y1,x2,y2);
        backBufferGraphics.drawLine(x1,y1,x2,y2);
    }

    public void drawOval(int x, int y, int width, int height)
    {
        graphics.drawOval(x,y,width,height);
        backBufferGraphics.drawOval(x,y,width,height);
    }

    public void drawPolygon(int[] xPoints, int[] yPoints, int nPoints)
    {
        graphics.drawPolygon(xPoints,yPoints,nPoints);
        backBufferGraphics.drawPolygon(xPoints,yPoints,nPoints);
    }

    public void drawPolygon(Polygon p)
    {
        graphics.drawPolygon(p);
        backBufferGraphics.drawPolygon(p);
    }

    public void drawPolyline(int[] xPoints, int[] yPoints, int nPoints)
    {
        graphics.drawPolyline(xPoints,yPoints,nPoints);
        backBufferGraphics.drawPolyline(xPoints,yPoints,nPoints);
    }

    public void drawRect(int x, int y, int width, int height)
    {
        graphics.drawRect(x,y,width,height);
        backBufferGraphics.drawRect(x,y,width,height);
    }

    public void drawRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight)
    {
        graphics.drawRoundRect(x,y,width,height,arcWidth,arcHeight);
        backBufferGraphics.drawRoundRect(x,y,width,height,arcWidth,arcHeight);
    }

    public void drawString(AttributedCharacterIterator iterator, int x, int y)
    {
        graphics.drawString(iterator,x,y);
        backBufferGraphics.drawString(iterator,x,y);
    }

    public void drawString(String str, int x, int y)
    {
        graphics.drawString(str,x,y);
        backBufferGraphics.drawString(str,x,y);
    }

    public void fill3DRect(int x, int y, int width, int height, boolean raised)
    {
        graphics.fill3DRect(x,y,width,height,raised);
        backBufferGraphics.fill3DRect(x,y,width,height,raised);
    }

    public void fillArc(int x, int y, int width, int height, int startAngle, int arcAngle)
    {
        graphics.fillArc(x,y,width,height,startAngle,arcAngle);
        backBufferGraphics.fillArc(x,y,width,height,startAngle,arcAngle);
    }

    public void fillOval(int x, int y, int width, int height)
    {
        graphics.fillOval(x,y,width,height);
        backBufferGraphics.fillOval(x,y,width,height);
    }

    public void fillPolygon(int[] xPoints, int[] yPoints, int nPoints)
    {
        graphics.fillPolygon(xPoints,yPoints,nPoints);
        backBufferGraphics.fillPolygon(xPoints,yPoints,nPoints);
    }

    public void fillPolygon(Polygon p)
    {
        graphics.fillPolygon(p);
        backBufferGraphics.fillPolygon(p);
    }

    public void fillRect(int x, int y, int width, int height)
    {
        graphics.fillRect(x,y,width,height);
        backBufferGraphics.fillRect(x,y,width,height);
    }

    public void fillRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight)
    {
        graphics.fillRoundRect(x,y,width,height,arcWidth,arcHeight);
        backBufferGraphics.fillRoundRect(x,y,width,height,arcWidth,arcHeight);
    }

    public Shape getClip()
    {
        return graphics.getClip();
    }

    public Rectangle getClipBounds()
    {
        return graphics.getClipBounds();
    }

    public Rectangle getClipBounds(Rectangle r)
    {
        return graphics.getClipBounds(r);
    }

    public Rectangle getClipRect()
    {
        return graphics.getClipRect();
    }

    public Color getColor()
    {
        return getColor();
    }

    public Font getFont()
    {
        return getFont();
    }

    public FontMetrics getFontMetrics()
    {
        return getFontMetrics();
    }

    public FontMetrics getFontMetrics(Font f)
    {
        return getFontMetrics(f);
    }

    public boolean hitClip(int x, int y, int width, int height)
    {
        return graphics.hitClip(x,y,width,height);
    }

    public void setClip(int x, int y, int width, int height)
    {
        graphics.setClip(x,y,width,height);
        backBufferGraphics.setClip(x,y,width,height);
    }

    public void setClip(Shape clip)
    {
        graphics.setClip(clip);
        backBufferGraphics.setClip(clip);
    }

    public void setColor(Color c)
    {
        graphics.setColor(c);
        backBufferGraphics.setColor(c);
    }

    public void setFont(Font font)
    {
        graphics.setFont(font);
        backBufferGraphics.setFont(font);
    }

    public void setPaintMode()
    {
        graphics.setPaintMode();
        backBufferGraphics.setPaintMode();
    }

    public void setXORMode(Color c1)
    {
        graphics.setXORMode(c1);
        backBufferGraphics.setXORMode(c1);
    }

    public String toString()
    {
        return graphics.toString();
    }

    public void translate(int x, int y)
    {
        graphics.translate(x,y);
        backBufferGraphics.translate(x,y);
    }

    //----------------------------------------------------------------------------------

    //
    // Graphics2D
    // ==========
    //
    public void addRenderingHints(Map hints)
    {
        graphics.addRenderingHints(hints);
        backBufferGraphics.addRenderingHints(hints);
    }

    public void clip(Shape s)
    {
        graphics.clip(s);
        backBufferGraphics.clip(s);
    }

    public void draw(Shape s)
    {
        graphics.draw(s);
        backBufferGraphics.draw(s);
    }

    public void drawGlyphVector(GlyphVector g, float x, float y)
    {
        graphics.drawGlyphVector(g,x,y);
        backBufferGraphics.drawGlyphVector(g,x,y);
    }

    public void drawImage(BufferedImage img, BufferedImageOp op, int x, int y)
    {
        graphics.drawImage(img,op,x,y);
        backBufferGraphics.drawImage(img,op,x,y);
    }

    public boolean drawImage(Image img, AffineTransform xform, ImageObserver obs)
    {
        backBufferGraphics.drawImage(img,xform,obs);
        return graphics.drawImage(img,xform,obs);
    }

    public void drawRenderableImage(RenderableImage img, AffineTransform xform)
    {
        graphics.drawRenderableImage(img,xform);
        backBufferGraphics.drawRenderableImage(img,xform);
    }

    public void drawRenderedImage(RenderedImage img, AffineTransform xform)
    {
        graphics.drawRenderedImage(img,xform);
        backBufferGraphics.drawRenderedImage(img,xform);
    }

    public void drawString(AttributedCharacterIterator iterator, float x, float y)
    {
        graphics.drawString(iterator,x,y);
        backBufferGraphics.drawString(iterator,x,y);
    }

    public void drawString(String s, float x, float y)
    {
        graphics.drawString(s,x,y);
        backBufferGraphics.drawString(s,x,y);
    }

    public void fill(Shape s)
    {
        graphics.fill(s);
        backBufferGraphics.fill(s);
    }

    public Color getBackground()
    {
        return graphics.getBackground();
    }

    public Composite getComposite()
    {
        return graphics.getComposite();
    }

    public GraphicsConfiguration getDeviceConfiguration()
    {
        return graphics.getDeviceConfiguration();
    }

    public FontRenderContext getFontRenderContext()
    {
        return graphics.getFontRenderContext();
    }

    public Paint getPaint()
    {
        return graphics.getPaint();
    }

    public Object getRenderingHint(RenderingHints.Key hintKey)
    {
        return graphics.getRenderingHint(hintKey);
    }

    public RenderingHints getRenderingHints()
    {
        return graphics.getRenderingHints();
    }

    public Stroke getStroke()
    {
        return graphics.getStroke();
    }

    public AffineTransform getTransform()
    {
        return graphics.getTransform();
    }

    public boolean hit(Rectangle rect, Shape s, boolean onStroke)
    {
        return graphics.hit(rect,s,onStroke);
    }

    public void rotate(double theta)
    {
        graphics.rotate(theta);
        backBufferGraphics.rotate(theta);
    }

    public void rotate(double theta, double x, double y)
    {
        graphics.rotate(theta,x,y);
        backBufferGraphics.rotate(theta,x,y);
    }

    public void scale(double sx, double sy)
    {
        graphics.scale(sx,sy);
        backBufferGraphics.scale(sx,sy);
    }

    public void setBackground(Color color)
    {
        graphics.setBackground(color);
        backBufferGraphics.setBackground(color);
    }

    public void setComposite(Composite comp)
    {
        graphics.setComposite(comp);
        backBufferGraphics.setComposite(comp);
    }

    public void setPaint(Paint paint)
    {
        graphics.setPaint(paint);
        backBufferGraphics.setPaint(paint);
    }

    public void setRenderingHint(RenderingHints.Key hintKey, Object hintValue)
    {
        graphics.setRenderingHint(hintKey,hintValue);
        backBufferGraphics.setRenderingHint(hintKey,hintValue);
    }

    public void setRenderingHints(Map hints)
    {
        graphics.setRenderingHints(hints);
        backBufferGraphics.setRenderingHints(hints);
    }

    public void setStroke(Stroke s)
    {
        graphics.setStroke(s);
        backBufferGraphics.setStroke(s);
    }

    public void setTransform(AffineTransform Tx)
    {
        graphics.setTransform(Tx);
        backBufferGraphics.setTransform(Tx);
    }

    public void shear(double shx, double shy)
    {
        graphics.shear(shx,shy);
        backBufferGraphics.shear(shx,shy);
    }

    public void transform(AffineTransform Tx)
    {
        graphics.transform(Tx);
        backBufferGraphics.transform(Tx);
    }

    public void translate(double tx, double ty)
    {
        graphics.translate(tx,ty);
        backBufferGraphics.translate(tx,ty);
    }
}
