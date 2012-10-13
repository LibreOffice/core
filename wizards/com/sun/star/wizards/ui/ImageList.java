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
package com.sun.star.wizards.ui;

import com.sun.star.awt.Key;
import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.MouseEvent;
import com.sun.star.awt.Size;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XItemEventBroadcaster;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XKeyListener;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.IRenderer;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.event.*;
import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

public class ImageList implements XItemEventBroadcaster, ListDataListener
{

    private XFixedText lblImageText;
    private XFixedText grbxSelectedImage;
    private XButton btnBack;
    private XButton btnNext;
    private XFixedText lblCounter;
    private XControl m_aImages[];
    private boolean benabled = true;
    private UnoDialog2 oUnoDialog;
    private Size gap = new Size(4, 4);
    private int cols = 4;
    private int rows = 3;
    private Size imageSize = new Size(20, 20);
    private Size pos;
    private Size selectionGap = new Size(2, 2);
    private boolean showButtons = true;
    private Short step;
    private final static Short NO_BORDER = new Short((short) 0);
    private boolean refreshOverNull = true;
    private int imageTextLines = 1;
    private boolean rowSelect = false;
    public int tabIndex;
    public Boolean scaleImages = Boolean.TRUE;
    public String name = "il";
    private int selected = -1;
    private int pageStart = 0;
    public int helpURL = 0;
    private CommonListener uiEventListener = new CommonListener();
    private IImageRenderer renderer;
    private ListModel listModel;
    public IRenderer counterRenderer = new SimpleCounterRenderer();
    private Object dialogModel;
    private ImageKeyListener imageKeyListener;
    private static final Integer BACKGROUND_COLOR = 16777216;
    private final static Short HIDE_PAGE = new Short((short) 99);
    private final static Integer TRANSPARENT = new Integer(-1);
    private final static int LINE_HEIGHT = 8;
    private MethodInvocation METHOD_MOUSE_PRESSED;

    /** Getter for property imageSize.
     * @return Value of property imageSize.
     *
     */
    public Size getImageSize()
    {
        return this.imageSize;
    }

    /** Setter for property imageSize.
     * @param imageSize New value of property imageSize.
     *
     */
    public void setImageSize(Size imageSize)
    {
        this.imageSize = imageSize;
    }

    class OMouseListener implements XMouseListener
    {
        public OMouseListener()
        {}
        public void mousePressed(MouseEvent arg0)
        {
            focus(getImageIndexFor(getSelected()));
        }

        public void mouseReleased(MouseEvent arg0)
        {
        }

        public void mouseEntered(MouseEvent arg0)
        {
        }

        public void mouseExited(MouseEvent arg0)
        {
        }

        public void disposing(EventObject arg0)
        {
        }
    }

    public void create(UnoDialog2 dialog)
    {
        oUnoDialog = dialog;
        dialogModel = dialog.xDialogModel;

        int imageTextHeight = imageTextLines * LINE_HEIGHT;

        PeerConfig opeerConfig = new PeerConfig(dialog);

        MOVE_SELECTION_VALS[2] = step;

        XControl imgContainer = dialog.insertImage(name + "lblContainer",
                new String[]
                {
                    "BackgroundColor",
                    PropertyNames.PROPERTY_BORDER,
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    BACKGROUND_COLOR,
                    new Short((short) 1),
                    new Integer((imageSize.Height + gap.Height) * rows + gap.Height + imageTextHeight + 1),
                    new Integer(pos.Width),
                    new Integer(pos.Height),
                    step,
                    new Integer((imageSize.Width + gap.Width) * cols + gap.Width)
                });

        opeerConfig.setPeerProperties(imgContainer, new String[]
                {
                    "MouseTransparent"
                }, new Object[]
                {
                    Boolean.TRUE
                });

        int selectionWidth = rowSelect ?
            (imageSize.Width + gap.Width) * cols - gap.Width + (selectionGap.Width * 2) :
            imageSize.Width + (selectionGap.Width * 2);

        grbxSelectedImage = dialog.insertLabel(name + "_grbxSelected", new String[]
                {
                    "BackgroundColor",
                    PropertyNames.PROPERTY_BORDER,
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    "Tabstop",
                    PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                    TRANSPARENT,
                    new Short((short) 1),
                    new Integer(imageSize.Height + (selectionGap.Height * 2)),
                    //height
                    0, //posx
                    0, //posy
                    step,
                    Boolean.TRUE,
                    new Integer(selectionWidth)
                });

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, grbxSelectedImage);
        xWindow.addMouseListener(new OMouseListener());

        final String[] pNames1 = new String[]
        {
            PropertyNames.PROPERTY_HEIGHT,
            PropertyNames.PROPERTY_HELPURL,
            PropertyNames.PROPERTY_POSITION_X,
            PropertyNames.PROPERTY_POSITION_Y,
            PropertyNames.PROPERTY_STEP,
            PropertyNames.PROPERTY_TABINDEX,
            "Tabstop",
            PropertyNames.PROPERTY_WIDTH
        };

        lblImageText = dialog.insertLabel(name + "_imageText", pNames1, new Object[]
                {
                    new Integer(imageTextHeight),
                    PropertyNames.EMPTY_STRING,
                    new Integer(pos.Width + 1),
                    new Integer(pos.Height + (imageSize.Height + gap.Height) * rows + gap.Height),
                    step,
                    new Short((short) 0),
                    Boolean.FALSE,
                    new Integer(cols * (imageSize.Width + gap.Width) + gap.Width - 2)
                });


        if (showButtons)
        {
            final Integer btnSize = 14;

            btnBack = dialog.insertButton(name + "_btnBack", "prevPage", this, pNames1, new Object[]
                    {
                        btnSize,
                        HelpIds.getHelpIdString(helpURL++),
                        new Integer(pos.Width),
                        new Integer(pos.Height + (imageSize.Height + gap.Height) * rows + gap.Height + imageTextHeight + 1),
                        step,
                        new Short((short) (tabIndex + 1)),
                        Boolean.TRUE,
                        btnSize
                    });

            btnNext = dialog.insertButton(name + "_btnNext", "nextPage", this, pNames1, new Object[]
                    {
                        btnSize,
                        HelpIds.getHelpIdString(helpURL++),
                        new Integer(pos.Width + (imageSize.Width + gap.Width) * cols + gap.Width - btnSize.intValue() + 1),
                        new Integer(pos.Height + (imageSize.Height + gap.Height) * rows + gap.Height + imageTextHeight + 1),
                        step,
                        new Short((short) (tabIndex + 2)),
                        Boolean.TRUE,
                        btnSize
                    });

            lblCounter = dialog.insertLabel(name + "_lblCounter", pNames1, new Object[]
                    {
                        new Integer(LINE_HEIGHT),
                        PropertyNames.EMPTY_STRING,
                        new Integer(pos.Width + btnSize.intValue() + 1),
                        new Integer(pos.Height + (imageSize.Height + gap.Height) * rows + gap.Height + imageTextHeight + ((btnSize.intValue() - LINE_HEIGHT) / 2)),
                        step,
                        new Short((short) 0),
                        Boolean.FALSE,
                        new Integer(cols * (imageSize.Width + gap.Width) + gap.Width - 2 * btnSize.intValue() - 1)
                    });

            Helper.setUnoPropertyValue(getModel(lblCounter), PropertyNames.PROPERTY_ALIGN, new Short((short) 1));
            Helper.setUnoPropertyValue(getModel(btnBack), PropertyNames.PROPERTY_LABEL, "<");
            Helper.setUnoPropertyValue(getModel(btnNext), PropertyNames.PROPERTY_LABEL, ">");


        }

        imageKeyListener = new ImageKeyListener();
        m_tabIndex = new Short((short) tabIndex);

        m_aImages = new XControl[rows * cols];

        try
        {
            METHOD_MOUSE_PRESSED = new MethodInvocation("mousePressed", this, Object.class);
        }
        catch (NoSuchMethodException e)
        {
            e.printStackTrace();
        }

        m_imageHeight = new Integer(imageSize.Height);
        m_imageWidth = new Integer(imageSize.Width);

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                m_aImages[r * cols + c] = createImage(dialog, r, c);
            }
        }
        refreshImages();

        listModel.addListDataListener(this);

    }
    private Integer m_imageHeight,  m_imageWidth;
    private final static String[] IMAGE_PROPS = new String[]
    {
        PropertyNames.PROPERTY_BORDER,
        "BackgroundColor",
        PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_HELPURL,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y,
        "ScaleImage",
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX,
        "Tabstop",
        PropertyNames.PROPERTY_WIDTH
    };
    //used for optimization
    private Short m_tabIndex;

    private XControl createImage(UnoDialog2 dialog, int _row, int _col)
                {
        String imageName = name + "_image" + (_row * cols + _col);
        XControl image = dialog.insertImage(imageName,
                IMAGE_PROPS,
                new Object[]
                {
                    NO_BORDER,
                    BACKGROUND_COLOR,
                    m_imageHeight,
                    HelpIds.getHelpIdString(helpURL++),
                    new Integer(getImagePosX(_col)),
                    new Integer(getImagePosY(_row)),
                    scaleImages,
                    step,
                    m_tabIndex,
                    Boolean.FALSE,
                    m_imageWidth
                });

        XWindow win = UnoRuntime.queryInterface(XWindow.class, image);
        win.addMouseListener(uiEventListener);
        win.addKeyListener(imageKeyListener);
        uiEventListener.add(imageName, EventNames.EVENT_MOUSE_PRESSED, METHOD_MOUSE_PRESSED);

        return image;
    }

    private int getImagePosX(int col)
    {
        return pos.Width + col * (imageSize.Width + gap.Width) + gap.Width;
    }

    private int getImagePosY(int row)
    {
        return pos.Height + row * (imageSize.Height + gap.Height) + gap.Height;
    }

    private void refreshImages()
    {
        if (showButtons)
        {
            refreshCounterText();
        }
        hideSelection();
        if (refreshOverNull)
        {
            for (int i = 0; i < m_aImages.length; i++)
            {
                setVisible(m_aImages[i], false);
            }
        }
        boolean focusable = true;
        for (int i = 0; i < m_aImages.length; i++)
        {
            Object[] oResources = renderer.getImageUrls(getObjectFor(i));
            if (oResources != null)
            {
                if (oResources.length == 1)
                {
                    Helper.setUnoPropertyValue(m_aImages[i].getModel(), PropertyNames.PROPERTY_IMAGEURL, oResources[0]);
                }
                else if (oResources.length == 2)
                {
                    oUnoDialog.getPeerConfiguration().setImageUrl(m_aImages[i].getModel(), oResources[0], oResources[1]);
                }
                Helper.setUnoPropertyValue(m_aImages[i].getModel(), "Tabstop", focusable ? Boolean.TRUE : Boolean.FALSE);
                if (refreshOverNull)
                {
                    setVisible(m_aImages[i], true);
                }
                focusable = false;
            }
        }
        refreshSelection();
    }

    private void refreshCounterText()
    {
        Helper.setUnoPropertyValue(getModel(lblCounter), PropertyNames.PROPERTY_LABEL, counterRenderer.render(new Counter(pageStart + 1, pageEnd(), listModel.getSize())));
    }

    private int pageEnd()
    {
        int i = pageStart + cols * rows;
        if (i > listModel.getSize() - 1)
        {
            return listModel.getSize();
        }
        else
        {
            return i;
        }
    }

    private void refreshSelection()
    {
        if (selected < pageStart || selected >= (pageStart + rows * cols))
        {
            hideSelection();
        }
        else
        {
            moveSelection(getImageIndexFor(selected));
        }
    }

    private void hideSelection()
    {
        Helper.setUnoPropertyValue(getModel(grbxSelectedImage), PropertyNames.PROPERTY_STEP, HIDE_PAGE);
        setVisible(grbxSelectedImage, false);
    }
    private final static String[] MOVE_SELECTION = new String[]
    {
        PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP
    };
    private Object[] MOVE_SELECTION_VALS = new Object[3];
    /** Utility field holding list of ItemListeners. */
    private transient java.util.ArrayList<XItemListener> itemListenerList;

    private void moveSelection(int image)
    {
        setVisible(grbxSelectedImage, false);

        int row = image / cols;
        int col = rowSelect ? 0 : image - (row * cols);

        MOVE_SELECTION_VALS[0] = new Integer(getImagePosX(col) - selectionGap.Width);
        MOVE_SELECTION_VALS[1] = new Integer(getImagePosY(row) - selectionGap.Height);

        Helper.setUnoPropertyValues(getModel(grbxSelectedImage), MOVE_SELECTION, MOVE_SELECTION_VALS);

        if (((Number) Helper.getUnoPropertyValue(dialogModel, PropertyNames.PROPERTY_STEP)).shortValue() == step.shortValue())
        {
            setVisible(grbxSelectedImage, true);        //now focus...
        }
        for (int i = 0; i < m_aImages.length; i++)
        {
            if (i != image)
            {
                defocus(i);
            }
            else
            {
                Helper.setUnoPropertyValue(m_aImages[image].getModel(), "Tabstop", Boolean.TRUE);
            }
        }
    }

    private void setVisible(Object control, boolean visible)
    {
        UnoRuntime.queryInterface(XWindow.class, control).setVisible(visible);
    }

    /**
     * 
     * @param i
     * @return the Object in the list model corresponding to the given image index.
     */
    private Object getObjectFor(int i)
    {
        int ii = getIndexFor(i);
        if (listModel.getSize() <= ii)
        {
            return null;
        }
        else
        {
            return listModel.getElementAt(ii);
        }
    }

    /**
     * 
     * @param i
     * @return the index in the listModel for the given image index.
     */
    private int getIndexFor(int i)
    {
        return pageStart + i;
    }

    private int getImageIndexFor(int i)
    {
        return i - pageStart;
    }

    public void contentsChanged(ListDataEvent event)
    {
        /*//if the content that was changed is in the displayed range, refresh...
        if (  (event.getIndex1() <  getIndexFor(0)) // range is before...
        || event.getIndex0() > getIndexFor( cols*rows - 1)) ; //do nothing
        else
        refreshImages();
         */        //selected = 0;
        //pageStart = 0;
        //if (event.refreshImages();
    }

    public void intervalAdded(ListDataEvent event)
    {
        if (event.getIndex0() <= selected)
        {
            if (event.getIndex1() <= selected)
            {
                selected += event.getIndex1() - event.getIndex0() + 1;
            }
        }
        if (event.getIndex0() < pageStart || event.getIndex1() < (pageStart + getRows() + getCols()))
        {
            refreshImages();
        }
    }

    public void intervalRemoved(ListDataEvent event)
    {
        //contentsChanged(event);
    }

    /** Registers ItemListener to receive events.
     * @param listener The listener to register.
     *
     */
    public synchronized void addItemListener(XItemListener listener)
    {
        if (itemListenerList == null)
        {
            itemListenerList = new java.util.ArrayList<XItemListener>();
        }
        itemListenerList.add(listener);
    }

    /** Removes ItemListener from the list of listeners.
     * @param listener The listener to remove.
     *
     */
    public synchronized void removeItemListener(XItemListener listener)
    {
        if (itemListenerList != null)
        {
            itemListenerList.remove(listener);
        }
    }

    /** Notifies all registered listeners about the event.
     */
    @SuppressWarnings("unchecked")
    private void fireItemSelected()
    {
        java.util.ArrayList<XItemListener> list;
        synchronized(this)
        {
            if (itemListenerList == null)
            {
                return;
            }
            list = (java.util.ArrayList<XItemListener>) itemListenerList.clone();
        }
        for (int i = 0; i < list.size(); i++)
        {
            list.get(i).itemStateChanged(null);
        }
    }

    /**
     * @return
     */
    public int getCols()
    {
        return cols;
    }

    public Size getGap()
    {
        return gap;
    }

    public ListModel getListModel()
    {
        return listModel;
    }

    public Short getStep()
    {
        return step;
    }

    public int getPageStart()
    {
        return pageStart;
    }

    public Size getPos()
    {
        return pos;
    }

    public IImageRenderer getRenderer()
    {
        return renderer;
    }

    public int getRows()
    {
        return rows;
    }

    public int getSelected()
    {
        return selected;
    }

    public Size getSelectionGap()
    {
        return selectionGap;
    }

    public boolean isShowButtons()
    {
        return showButtons;
    }

    public void setCols(int i)
    {
        cols = i;
    }

    public void setGap(Size size)
    {
        gap = size;
    }

    public void setListModel(ListModel model)
    {
        listModel = model;
    }

    public void setStep(Short short1)
    {
        step = short1;
    }

    public void setPageStart(int i)
    {
        if (i == pageStart)
        {
            return;
        }
        pageStart = i;
        enableButtons();
        refreshImages();
    }

    public void setPos(Size size)
    {
        pos = size;
    }

    public void setRenderer(IImageRenderer renderer)
    {
        this.renderer = renderer;
    }

    public void setRows(int i)
    {
        rows = i;
    }

    public void setSelected(int i)
    {
        if (rowSelect && (i >= 0))
        {
            i = (i / cols) * cols;
        }
        if (selected == i)
        {
            return;
        }
        selected = i;
        refreshImageText();
        refreshSelection();
        fireItemSelected();
    }

    public void setSelected(Object object)
    {
        if (object == null)
        {
            setSelected(-1);
        }
        else
        {
            for (int i = 0; i < getListModel().getSize(); i++)
            {
                if (getListModel().getElementAt(i).equals(object))
                {
                    setSelected(i);
                    return;
                }
            }
        }
        setSelected(-1);

    }

    private void refreshImageText()
    {
        Object item = selected >= 0 ? getListModel().getElementAt(selected) : null;
        Helper.setUnoPropertyValue(getModel(lblImageText), PropertyNames.PROPERTY_LABEL, PropertyNames.SPACE + renderer.render(item));
    }

    public void setSelectionGap(Size size)
    {
        selectionGap = size;
    }

    public void setShowButtons(boolean b)
    {
        showButtons = b;
    }

    public void nextPage()
    {
        if (pageStart < getListModel().getSize() - rows * cols)
        {
            setPageStart(pageStart + rows * cols);
        }
    }

    public void prevPage()
    {
        if (pageStart == 0)
        {
            return;
        }
        int i = pageStart - rows * cols;
        if (i < 0)
        {
            i = 0;
        }
        setPageStart(i);
    }

    private void enableButtons()
    {
        enable(btnNext, Boolean.valueOf(pageStart + rows * cols < listModel.getSize()));
        enable(btnBack, Boolean.valueOf(pageStart > 0));
    }

    private void enable(Object control, Boolean enable)
    {
        Helper.setUnoPropertyValue(getModel(control), PropertyNames.PROPERTY_ENABLED, enable);
    }

    private Object getModel(Object control)
    {
        return UnoRuntime.queryInterface(XControl.class, control).getModel();
    }

    //TODO what is when the image does not display an image?
    private void setBorder(Object control, Short border)
    {
        Helper.setUnoPropertyValue(getModel(control), PropertyNames.PROPERTY_BORDER, border);
    }

    private int getImageFromEvent(Object event)
    {
        Object image = ((EventObject) event).Source;
        String controlName = (String) Helper.getUnoPropertyValue(getModel(image), PropertyNames.PROPERTY_NAME);
        return Integer.valueOf(controlName.substring(6 + name.length())).intValue();

    }

    public void mousePressed(Object event)
    {
        int image = getImageFromEvent(event);
        int index = getIndexFor(image);
        if (index < listModel.getSize())
        {
            focus(image);
            setSelected(index);
        }
    }

    public Object[] getSelectedObjects()
    {
        return new Object[]
                {
                    getListModel().getElementAt(selected)
                };
    }

    public static interface IImageRenderer extends IRenderer
    {

        /**
         * @param listItem
         * @return two resource ids for an image referenced in the imaglist resourcefile of the 
         * wizards project; The second one of them is designed to be used for High Contrast Mode.
         */
        public Object[] getImageUrls(Object listItem);
    }

    private static class SimpleCounterRenderer implements IRenderer
    {

        public String render(Object counter)
        {
            return PropertyNames.EMPTY_STRING + ((Counter) counter).start + ".." + ((Counter) counter).end + "/" + ((Counter) counter).max;
        }
    }

    public static class Counter
    {

        public int start,  end,  max;

        public Counter(int start_, int end_, int max_)
        {
            start = start_;
            end = end_;
            max = max_;
        }
    }

    public Object getSelectedObject()
    {
        return selected >= 0 ? getListModel().getElementAt(selected) : null;
    }

    public void showSelected()
    {
        int oldPageStart = pageStart;
        if (selected == -1)
        {
            pageStart += 0;
        }
        else
        {
            pageStart = (selected / m_aImages.length) * m_aImages.length;
        }
        if (oldPageStart != pageStart)
        {
            enableButtons();
            refreshImages();
        }
    }

    public void setRowSelect(boolean b)
    {
        rowSelect = b;
    }

    public boolean isRowSelect()
    {
        return rowSelect;
    }

    private class ImageKeyListener implements XKeyListener
    {

        /* (non-Javadoc)
         * @see com.sun.star.awt.XKeyListener#keyPressed(com.sun.star.awt.KeyEvent)
         */
        public void keyPressed(KeyEvent ke)
        {
            int image = getImageFromEvent(ke);
            int r = image / getCols();
            int c = image - (r * getCols());
            int d = getKeyMove(ke, r, c);
            int newImage = image + d;
            if (newImage == image)
            {
                return;
            }
            if (isFocusable(newImage))
            {
                changeFocus(image, newImage);
            }
        }

        private boolean isFocusable(int image)
        {
            return (image >= 0) && (getIndexFor(image) < listModel.getSize());
        }

        private void changeFocus(int oldFocusImage, int newFocusImage)
        {
            focus(newFocusImage);
            defocus(oldFocusImage);
        }

        private final int getKeyMove(KeyEvent ke, int row, int col)
        {
            switch (ke.KeyCode)
            {
                case Key.UP:
                    if (row > 0)
                    {
                        return 0 - getCols();
                    }
                    break;
                case Key.DOWN:
                    if (row < getRows() - 1)
                    {
                        return getCols();
                    }
                    break;
                case Key.LEFT:
                    if (col > 0)
                    {
                        return -1;
                    }
                    break;
                case Key.RIGHT:
                    if (col < getCols() - 1)
                    {
                        return 1;
                    }
                    break;
                case Key.SPACE:
                    select(ke);
            }
            return 0;
        }

        private void select(KeyEvent ke)
        {
            setSelected(getIndexFor(getImageFromEvent(ke)));
        }

        public void keyReleased(KeyEvent ke)
        {
        }

        public void disposing(EventObject arg0)
        {
        }
    }

    private final void focus(int image)
    {
        Helper.setUnoPropertyValue(m_aImages[image].getModel(), "Tabstop",
                Boolean.TRUE);
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, m_aImages[image]);
        xWindow.setFocus();
    }

    private final void defocus(int image)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(m_aImages[image]), "Tabstop",
                Boolean.FALSE);

    }

    /**
     * jump to the given item (display the screen
     * that contains the given item).
     * @param i
     */
    public void display(int i)
    {
        int is = (getCols() * getRows());
        int ps = (listModel.getSize() / is) * is;
        setPageStart(ps);
    }

    public boolean isenabled()
    {
        return benabled;
    }

    public void setenabled(boolean b)
    {

        for (int i = 0; i < m_aImages.length; i++)
        {
            UnoDialog2.setEnabled(m_aImages[i], b);
        }
        UnoDialog2.setEnabled(grbxSelectedImage, b);
        UnoDialog2.setEnabled(lblImageText, b);
        if (showButtons)
        {
            UnoDialog2.setEnabled(btnBack, b);
            UnoDialog2.setEnabled(btnNext, b);
            UnoDialog2.setEnabled(lblCounter, b);
        }
        benabled = b;
    }
}
