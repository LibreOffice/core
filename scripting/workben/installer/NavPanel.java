package installer;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
public class NavPanel extends JPanel implements ActionListener {

    NavPanel(InstallWizard wizard, boolean bBack, boolean bNext, boolean bCancel, String prev, String next) {
        setBackground(Color.white);
        setBorder(new javax.swing.border.EtchedBorder(javax.swing.border.EtchedBorder.LOWERED));
        this.wizard = wizard;
        this.next = next;
        this.prev = prev;
        navBack = new javax.swing.JButton("<< Back");
        navNext = new javax.swing.JButton("Next >>");
        navCancel = new javax.swing.JButton("Cancel");
        setLayout(new GridBagLayout());

        gridBagConstraints1 = new java.awt.GridBagConstraints();
        gridBagConstraints1.insets = new java.awt.Insets(1, 1, 1, 1);
      gridBagConstraints1.anchor = gridBagConstraints1.WEST;

        gridBagConstraints2 = new java.awt.GridBagConstraints();
        gridBagConstraints2.gridx = 2;
        gridBagConstraints2.gridy = 0;

        gridBagConstraints3 = new java.awt.GridBagConstraints();
        gridBagConstraints3.gridx = 6;
        gridBagConstraints3.gridy = 0;

        navNext.setEnabled(bNext);
        navBack.setEnabled(bBack);
        navCancel.setEnabled(bCancel);
        navNext.addActionListener(this);
        navBack.addActionListener(this);
        navCancel.addActionListener(this);
        add(navBack, gridBagConstraints1);
        add(navNext, gridBagConstraints2);
        add(navCancel, gridBagConstraints3);
    }

    public void enableNext(boolean bEnable) {
        navNext.setEnabled(bEnable);
    }

    public void enableBack(boolean bEnable) {
        navBack.setEnabled(bEnable);
    }

    public void enableCancel(boolean bEnable) {
        navCancel.setEnabled(bEnable);
    }

    public void enableIDE(boolean bEnable) {
    ideDetected = bEnable;
    }

    public void actionPerformed(ActionEvent ev) {
        if ((ev.getSource() == navNext) && (next.length() != 0)) {
            wizard.show(next);
        }
        if ((ev.getSource() == navBack) && (prev.length() != 0)) {
            wizard.show(prev);
        }
        if (ev.getSource() == navCancel) {
        if( ideDetected ) {
            wizard.show(InstallWizard.IDEWELCOME);
        }
        else {
            wizard.exitForm(null);
        }
        enableIDE(false);
        }
    }

    public void setNextListener(ActionListener listener) {
        navNext.addActionListener(listener);
    }

    public void setBackListener(ActionListener listener) {
        navBack.addActionListener(listener);
    }

    public void setCancelListener(ActionListener listener) {
        navCancel.addActionListener(listener);
    }

    public void removeNextListener(ActionListener listener)
    {
        navNext.removeActionListener(listener);
    }

    public void removeBackListener(ActionListener listener)
    {
        navBack.removeActionListener(listener);
    }

    public void removeCancelListener(ActionListener listener)
    {
        navCancel.removeActionListener(listener);
    }

    public JButton navBack;
    public JButton navNext;
    public JButton navCancel;
    private GridBagConstraints gridBagConstraints1;
    private GridBagConstraints gridBagConstraints2;
    private GridBagConstraints gridBagConstraints3;
    private InstallWizard wizard;
    private String next;
    private String prev;
    private boolean ideDetected = false;
}
