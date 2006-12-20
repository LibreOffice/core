package org.openoffice.setup.Util;

import javax.swing.JOptionPane;

public class Informer {

    private Informer() {
    }

    static public void showErrorMessage(String message, String title) {
        JOptionPane.showMessageDialog(null, message, title, JOptionPane.ERROR_MESSAGE );
    }

    static public void showInfoMessage(String message, String title) {
        JOptionPane.showMessageDialog(null, message, title, JOptionPane.INFORMATION_MESSAGE );
    }

}
