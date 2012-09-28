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

import com.sun.star.awt.Size;
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XItemEventBroadcaster;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.IRenderer;
import com.sun.star.wizards.common.PropertySetHelper;
import com.sun.star.wizards.common.PropertyNames;
import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;
import com.sun.star.wizards.common.HelpIds;

/**
 *
 * @author  rpiterman,ll93751
 */
public class ButtonList implements XItemEventBroadcaster, XActionListener
{

    private XFixedText lblImageText;
    private XButton btnBack;
    private XButton btnNext;
    private XFixedText lblCounter;
    private XControl m_aButtons[];
    private boolean benabled = true;
    private UnoDialog2 oUnoDialog;
    private Size gap = new Size(4, 4);
    private int cols = 4;
    private int rows = 3;
    private Size m_aButtonSize = new Size(20, 20);
    private Size pos;
    private Size selectionGap = new Size(2, 2);
    private boolean showButtons = true;
    private Short step;
    private boolean refreshOverNull = true;
    private int imageTextLines = 1;
    private boolean rowSelect = false;
    public int tabIndex;
    public Boolean scaleImages = Boolean.TRUE;
    private String m_aControlName = "il";
    private int m_nCurrentSelection = -1;
    private int pageStart = 0;
    public int helpURL = 0;
    private IImageRenderer renderer;
    private ListModel listModel;
    public IRenderer counterRenderer = new SimpleCounterRenderer();
    private final static int LINE_HEIGHT = 8;    //private MethodInvocation METHOD_MOUSE_ENTER_IMAGE;

    /** Getter for property m_aButtonSize.
     * @return Value of property m_aButtonSize.
     *
     */
    public Size getButtonSize()
    {
        return this.m_aButtonSize;
    }
    public void setName(String _sName)
    {
        m_aControlName = _sName;
    }
    
    /** Setter for property m_aButtonSize.
     * @param imageSize  New value of property m_aButtonSize.
     */
    public void setButtonSize(Size imageSize)
    {
        this.m_aButtonSize = imageSize;
    }

//    @Override
    public void disposing(EventObject arg0)
    {
        // throw new UnsupportedOperationException("Not supported yet.");
    }

    public void create(UnoDialog2 dialog)
    {
        oUnoDialog = dialog;

        int imageTextHeight = imageTextLines * LINE_HEIGHT;

        int nXPos = pos.Width + 1;
        int nYPos = pos.Height + (m_aButtonSize.Height + gap.Height) * rows + gap.Height;
        int nWidth = cols * (m_aButtonSize.Width + gap.Width) + gap.Width - 2;

        lblImageText = dialog.insertLabel(m_aControlName + "_imageText",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_HELPURL,
                    PropertyNames.PROPERTY_LABEL,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    "Tabstop",
                    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Integer.valueOf(imageTextHeight),
                    PropertyNames.EMPTY_STRING,
                    "(1)",
                    Integer.valueOf(nXPos),
                    Integer.valueOf(nYPos),
                    step,
                    Short.valueOf((short) 0),
                    Boolean.FALSE,
                    Integer.valueOf(nWidth)
                });


        if (showButtons)
        {
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

            final Integer btnSize = Integer.valueOf(14);

// TODO: if list of strings not the same length of list object, office will die.
            btnBack = dialog.insertButton(m_aControlName + "_btnBack", "prevPage", this, pNames1, new Object[]
                    {
                        btnSize,
                        HelpIds.getHelpIdString(helpURL++),
                        Integer.valueOf(pos.Width),
                        Integer.valueOf(pos.Height + (m_aButtonSize.Height + gap.Height) * rows + gap.Height + imageTextHeight + 1),
                        step,
                        Short.valueOf((short) (tabIndex + 1)),
                        Boolean.TRUE,
                        btnSize
                    });

            btnNext = dialog.insertButton(m_aControlName + "_btnNext", "nextPage", this, pNames1, new Object[]
                    {
                        btnSize,
                        HelpIds.getHelpIdString(helpURL++),
                        Integer.valueOf(pos.Width + (m_aButtonSize.Width + gap.Width) * cols + gap.Width - btnSize.intValue() + 1),
                        Integer.valueOf(pos.Height + (m_aButtonSize.Height + gap.Height) * rows + gap.Height + imageTextHeight + 1),
                        step,
                        new Short((short) (tabIndex + 2)),
                        Boolean.TRUE,
                        btnSize
                    });

            lblCounter = dialog.insertLabel(m_aControlName + "_lblCounter", pNames1, new Object[]
                    {
                        Integer.valueOf(LINE_HEIGHT),
                        PropertyNames.EMPTY_STRING,
                        Integer.valueOf(pos.Width + btnSize.intValue() + 1),
                        Integer.valueOf(pos.Height + (m_aButtonSize.Height + gap.Height) * rows + gap.Height + imageTextHeight + ((btnSize.intValue() - LINE_HEIGHT) / 2)),
                        step,
                        Short.valueOf((short) 0),
                        Boolean.FALSE,
                        Integer.valueOf(cols * (m_aButtonSize.Width + gap.Width) + gap.Width - 2 * btnSize.intValue() - 1)
                    });

            Helper.setUnoPropertyValue(getModel(lblCounter), PropertyNames.PROPERTY_ALIGN, new Short((short) 1));
            Helper.setUnoPropertyValue(getModel(btnBack), PropertyNames.PROPERTY_LABEL, "<");
            Helper.setUnoPropertyValue(getModel(btnNext), PropertyNames.PROPERTY_LABEL, ">");


        }

        m_tabIndex = new Short((short) tabIndex);

        m_aButtons = new XControl[rows * cols];


        m_aButtonHeight = Integer.valueOf(m_aButtonSize.Height);
        m_aButtonWidth = Integer.valueOf(m_aButtonSize.Width);

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                XButton aButton = createButton(dialog, r, c);
                XControl aControl = UnoRuntime.queryInterface(XControl.class, aButton);
                m_aButtons[r * cols + c] = aControl;
            }
        }
        refreshImages();
    }

    private Integer m_aButtonHeight;
    private Integer m_aButtonWidth;
    private Short m_tabIndex;

    private XButton createButton(UnoDialog2 dialog, int _row, int _col)
    {
        String sButtonName = m_aControlName + "_button" + (_row * cols + _col);
        int nButtonX = getButtonPosX(_col);
        int nButtonY = getButtonPosY(_row);
        XButton aButton = dialog.insertImageButton(sButtonName, this,
                new String[]
                {
                    /* PropertyNames.PROPERTY_BORDER, */
                    /* "BackgroundColor", */
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_HELPURL,
                    /* PropertyNames.PROPERTY_LABEL, */
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    /* "ScaleImage", */
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    "Tabstop",
                    "Toggle",
                    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    /* Short.valueOf((short) 1), */ /* NO_BORDER, */
                    /* BACKGROUND_COLOR, */
                    m_aButtonHeight,
                    HelpIds.getHelpIdString(helpURL++),
                    /* "Test", */
                    Integer.valueOf(nButtonX),
                    Integer.valueOf(nButtonY),
                    /* scaleImages, */
                    step,
                    m_tabIndex,
                    Boolean.TRUE,
                    Boolean.TRUE, /* Toggle */
                    m_aButtonWidth
                });

        XWindow win = UnoRuntime.queryInterface(XWindow.class, aButton);
        win.setEnable(true);
        win.setVisible(true);
        return aButton;
    }

    private int getButtonPosX(int _col)
    {
        return pos.Width + _col * (m_aButtonSize.Width + gap.Width) + gap.Width;
    }

    private int getButtonPosY(int _row)
    {
        return pos.Height + _row * (m_aButtonSize.Height + gap.Height) + gap.Height;
    }

    private void refreshImages()
    {
        if (showButtons)
        {
            refreshCounterText();
        }
        if (refreshOverNull)
        {
            for (int i = 0; i < m_aButtons.length; i++)
            {
                setVisible(m_aButtons[i], false);
            }
        }
        boolean focusable = true;
        for (int i = 0; i < m_aButtons.length; i++)
        {
            Object[] oResources = renderer.getImageUrls(getObjectFor(i));
            if (oResources != null)
            {
                if (oResources.length == 1)
                {
                    Helper.setUnoPropertyValue(m_aButtons[i].getModel(), PropertyNames.PROPERTY_IMAGEURL, oResources[0]);
                }
                else if (oResources.length == 2)
                {
                    oUnoDialog.getPeerConfiguration().setImageUrl(m_aButtons[i].getModel(), oResources[0], oResources[1]);
//                    Helper.setUnoPropertyValue(m_aButtons[i].getModel(), PropertyNames.PROPERTY_IMAGEURL, oResources[0]);
                }
                boolean bTabStop = Boolean.TRUE; // focusable ? Boolean.TRUE : Boolean.FALSE;
                Helper.setUnoPropertyValue(m_aButtons[i].getModel(), "Tabstop", bTabStop);
                // Object aEnabled = Helper.getUnoPropertyValue(m_aButtons[i].getModel(), PropertyNames.PROPERTY_ENABLED);
                if (refreshOverNull)
                {
                    setVisible(m_aButtons[i], true);
                }
                focusable = false;
            }
        }
//        refreshSelection();
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

    /** Utility field holding list of ItemListeners. */
    private transient java.util.ArrayList<XItemListener> m_aItemListenerList;
    private void setVisible(Object control, boolean visible)
    {
        final XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, control);
        xWindow.setVisible(visible);
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
         */        //m_nCurrentSelection = 0;
        //pageStart = 0;
        //if (event.refreshImages();
    }

    public void intervalAdded(ListDataEvent event)
    {
        if (event.getIndex0() <= m_nCurrentSelection)
        {
            if (event.getIndex1() <= m_nCurrentSelection)
            {
                m_nCurrentSelection += event.getIndex1() - event.getIndex0() + 1;
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
    // @Override
    public synchronized void addItemListener(XItemListener listener)
    {
        if (m_aItemListenerList == null)
        {
            m_aItemListenerList = new java.util.ArrayList<XItemListener>();
        }
        m_aItemListenerList.add(listener);
    }

    /** Removes ItemListener from the list of listeners.
     * @param listener The listener to remove.
     *
     */
    public synchronized void removeItemListener(XItemListener listener)
    {
        if (m_aItemListenerList != null)
        {
            m_aItemListenerList.remove(listener);
        }
    }

    /** Notifies all registered listeners about the event.
     *
     */
    @SuppressWarnings("unchecked")
    private void fireItemSelected()
    {
//      java.awt.event.ItemEvent event = new java.awt.event.ItemEvent(this, 0, 
//          getSelectedObject(), java.awt.event.ItemEvent.SELECTED);
        java.util.ArrayList<XItemListener> list;
        synchronized(this)
        {
            if (m_aItemListenerList == null)
            {
                return;
            }
            list = (java.util.ArrayList<XItemListener>) m_aItemListenerList.clone();
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

    /**
     * @return
     */
    public Size getGap()
    {
        return gap;
    }

    /**
     * @return
     */
    public ListModel getListModel()
    {
        return listModel;
    }

    /**
     * @return
     */
    public Short getStep()
    {
        return step;
    }

    /**
     * @return
     */
    public int getPageStart()
    {
        return pageStart;
    }

    /**
     * @return
     */
    public Size getPos()
    {
        return pos;
    }

    /**
     * @return
     */
    public IImageRenderer getRenderer()
    {
        return renderer;
    }

    /**
     * @return
     */
    public int getRows()
    {
        return rows;
    }

    /**
     * @return
     */
    public int getSelected()
    {
        return m_nCurrentSelection;
    }

    /**
     * @return
     */
    public Size getSelectionGap()
    {
        return selectionGap;
    }

    /**
     * @return
     */
    public boolean isShowButtons()
    {
        return showButtons;
    }

    /**
     * @param i
     */
    public void setCols(int i)
    {
        cols = i;
    }

    /**
     * @param size
     */
    public void setGap(Size size)
    {
        gap = size;
    }

    /**
     * @param model
     */
    public void setListModel(ListModel model)
    {
        listModel = model;
    }

    /**
     * @param short1
     */
    public void setStep(Short short1)
    {
        step = short1;
    }

    /**
     * @param i
     */
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

    /**
     * @param _size
     */
    public void setPos(Size _size)
    {
        pos = _size;
    }

    /**
     * @param _renderer
     */
    public void setRenderer(IImageRenderer _renderer)
    {
        this.renderer = _renderer;
    }

    /**
     * @param i
     */
    public void setRows(int i)
    {
        rows = i;
    }

    /**
     * @param i
     */
    public void setSelected(int i)
    {
        if (rowSelect && (i >= 0))
        {
            i = (i / cols) * cols;
        }
        if (m_nCurrentSelection == i)
        {
            return;
        }
        m_nCurrentSelection = i;
        refreshImageText();
        refreshSelection();
        fireItemSelected();
    }

/*
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
*/

    /**
     * set the text under the button list
     */
    private void refreshImageText()
    {
        Object item = m_nCurrentSelection >= 0 ? getListModel().getElementAt(m_nCurrentSelection) : null;
        final String sText = PropertyNames.SPACE + renderer.render(item);
        Helper.setUnoPropertyValue(getModel(lblImageText), PropertyNames.PROPERTY_LABEL, sText);
    }

    /**
     * @param size
     */
    public void setSelectionGap(Size size)
    {
        selectionGap = size;
    }

    /**
     * @param b
     */
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

    private void setBorder(Object control, Short border)
    {
        Helper.setUnoPropertyValue(getModel(control), PropertyNames.PROPERTY_BORDER, border);
    //XWindowPeer peer = ((XControl)UnoRuntime.queryInterface(XControl.class,control)).getPeer();
    //peer.invalidate(InvalidateStyle.CHILDREN);
    }


    public Object[] getSelectedObjects()
    {
        return new Object[]
                {
                    getListModel().getElementAt(m_nCurrentSelection)
                };
    }


    /**
     */
    public static interface IImageRenderer extends IRenderer
    {

        /**
         * 
         * @param listItem
         * @return two resource ids for an image referenced in the imaglist resourcefile of the 
         * wizards project; The second one of them is designed to be used for High Contrast Mode.
         */
        public Object[] getImageUrls(Object listItem);
    }

    private static class SimpleCounterRenderer implements IRenderer
    {

        // @Override
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
        return m_nCurrentSelection >= 0 ? getListModel().getElementAt(m_nCurrentSelection) : null;
    }


    public void setRowSelect(boolean b)
    {
        rowSelect = b;
    }

    public boolean isRowSelect()
    {
        return rowSelect;
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

    /**
     * @return
     */
    public boolean isenabled()
    {
        return benabled;
    }

    /**
     * @param b
     */
    public void setenabled(boolean b)
    {

        for (int i = 0; i < m_aButtons.length; i++)
        {
            UnoDialog2.setEnabled(m_aButtons[i], b);
        }
//         UnoDialog2.setEnabled(grbxSelectedImage, b);
        UnoDialog2.setEnabled(lblImageText, b);
        if (showButtons)
        {
            UnoDialog2.setEnabled(btnBack, b);
            UnoDialog2.setEnabled(btnNext, b);
            UnoDialog2.setEnabled(lblCounter, b);
        }
        benabled = b;
    }

/**
 * refresh on all buttons, the selected button will get state pressed
 */
    private void refreshSelection()
    {
        // reset all buttons to not set
        for (int i = 0; i < m_aButtons.length; i++)
        {
            if (m_aButtons[i] != null)
            {
                XControlModel xModel = (XControlModel)UnoDialog2.getModel(m_aButtons[i]);
                PropertySetHelper aHelper = new PropertySetHelper(xModel);
                if (i == m_nCurrentSelection)
                {
                    final short one = 1;
                    aHelper.setPropertyValueDontThrow(PropertyNames.PROPERTY_STATE, Short.valueOf(one));
                }
                else
                {
                    final short zero = 0;
                    aHelper.setPropertyValueDontThrow(PropertyNames.PROPERTY_STATE, Short.valueOf(zero));
                }
            }
        }
    }

/**
 * implementation of XActionListener
 * will call if a button from the m_aButtonList is pressed.
 * @param actionEvent
 */
//    @Override
    public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
    {
        // bStopProcess = true;
        XControlModel xModel = (XControlModel)UnoDialog2.getModel(actionEvent.Source);
        PropertySetHelper aHelper = new PropertySetHelper(xModel);

        int nState = aHelper.getPropertyValueAsInteger(PropertyNames.PROPERTY_STATE, -1);
        if (nState == 0)
        {
            // this will avoid a wrong state, if already pressed.
            aHelper.setPropertyValueDontThrow(PropertyNames.PROPERTY_STATE, Short.valueOf((short)1));
        }

        // check which Button is pressed.
        String sControlName = aHelper.getPropertyValueAsString(PropertyNames.PROPERTY_NAME, PropertyNames.EMPTY_STRING);
        final String sButton = sControlName.substring(7 + m_aControlName.length());
        int nButton = Integer.parseInt(sButton);

        // int image = getImageFromEvent(event);
        int index = getIndexFor(nButton);
        if (index < listModel.getSize())
        {
            // focus(image);
            setSelected(index);
        }

    }


}
