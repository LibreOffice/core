/*
 * FieldSelectionActions.java
 *
 * Created on July 14, 2003, 5:42 PM
 */
package com.sun.star.wizards.ui;

/**
 *
 * @author  bc93774
 */
public interface XFieldSelectionListener {
    public void shiftFromLeftToRight(String[] SelItems, String[] NewItems);
    public void shiftFromRightToLeft(String[] OldSelItems, String[] NewItems);
    public void moveItemUp(String Selitem);
    public void moveItemDown(String Selitem);
    public void setID(String sIncSuffix);
    public int getID();
}
