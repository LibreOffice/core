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

import com.sun.star.reflection.ParamInfo;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.TypeClass;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.Vector;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

/**
 * @author bc93774
 */
public class MethodParametersDialog extends JDialog{
    private javax.swing.JPanel jPnlParamContainer;
    private ParameterPanel[] m_aParameterPanels;
    private ParamInfo[] m_aParamInfo;
    private XIdlMethod m_xIdlMethod;
    private ActionListener oActionListener;
    private JButton jHelpButton = new JButton("Help");
    private JButton jOKButton = new JButton("Ok");
    private JButton jInvokeButton = new JButton("Invoke");
    private Object m_oReturnButton = null;
    private Object m_oUnoObject = null;
    private Object m_oUnoReturnObject = null;
    private JLabel jLblResult;
    private JPanel jResultPanel = null;
    private boolean bisdiposed = false;
    private XUnoMethodNode m_xUnoMethodNode;


    public MethodParametersDialog(XUnoMethodNode _xUnoMethodNode){
        m_xUnoMethodNode = _xUnoMethodNode;
        m_xIdlMethod = _xUnoMethodNode.getXIdlMethod();
        m_aParamInfo = m_xIdlMethod.getParameterInfos();
        m_oUnoObject = m_xUnoMethodNode.getUnoObject();
        Object[] m_aParameterObjects = new Object[m_aParamInfo.length];
    }


    public Vector getMethodObjects() {
        super.setModal(true);
        addBorderPanel(getContentPane(), BorderLayout.NORTH);
        addBorderPanel(getContentPane(), BorderLayout.WEST);
        addBorderPanel(getContentPane(), BorderLayout.EAST);
        jPnlParamContainer = new JPanel();
        jPnlParamContainer.setLayout(new javax.swing.BoxLayout(jPnlParamContainer, javax.swing.BoxLayout.Y_AXIS));
        JPanel jHeaderPanel = new JPanel(new BorderLayout());
        JLabel jLblHeader = new JLabel();
        jLblHeader.setText("Please insert the values for the given Parameters of the method '" + m_xIdlMethod.getName() + "'");
        jHeaderPanel.add(jLblHeader,BorderLayout.WEST);
        jPnlParamContainer.add(jHeaderPanel);

        m_aParameterPanels = new ParameterPanel[m_aParamInfo.length];
        for (int i = 0; i < m_aParameterPanels.length; i++){
            m_aParameterPanels[i] = new ParameterPanel(m_aParamInfo[i]);
            jPnlParamContainer.add(m_aParameterPanels[i]);
        }
        jPnlParamContainer.add(new ButtonPanel(), java.awt.BorderLayout.SOUTH);
        getContentPane().add(jPnlParamContainer, java.awt.BorderLayout.CENTER);
        pack();
        setLocation(350, 350);
        setTitle("Object Inspector - Parameter Values of '" + m_xIdlMethod.getName() + "'");
        super.setFocusable(true);
        super.setFocusableWindowState(true);
        super.requestFocus();
        m_aParameterPanels[0].getInputComponent().requestFocusInWindow();
        setVisible(true);
        if (!bisdiposed){
            Vector aMethodObjects = new Vector();
            for (int i = 0; i < m_aParameterPanels.length; i++){
                aMethodObjects.add(m_aParameterPanels[i].getValue());
            }
            aMethodObjects.add(m_oUnoReturnObject);
            return aMethodObjects;
        }
        else{
            return null;
        }
    }


    private void insertResultPanel(Exception _oInvocationException){
        boolean bAddPanel = false;
        if (jResultPanel == null){
            jResultPanel = new JPanel(new BorderLayout());
            bAddPanel = true;
        }
        else{
            jResultPanel.removeAll();
        }
        jLblResult = new JLabel();
        jLblResult.setMaximumSize(new java.awt.Dimension(getSize().width - 20, 57));
        if (_oInvocationException != null){
            jLblResult.setText("<html>Invoking the method cause an exception: <br>" + _oInvocationException.toString() + "</html>");
        }
        else{
            jLblResult.setText("<html>The invocation of the method did not produce any error</html>");
        }
        jResultPanel.add(jLblResult,BorderLayout.WEST);
        if (bAddPanel){
            int nPos = jPnlParamContainer.getComponentCount() - 1;
            jPnlParamContainer.add(jResultPanel, nPos);
        }
        super.pack();
        super.validate();
    }


    private Object[] getParameterValues(){
        Object[] oParameterValues = new Object[m_aParameterPanels.length];
        for (int i = 0; i < m_aParameterPanels.length; i++){
            oParameterValues[i] = m_aParameterPanels[i].getValue();
        }
        return oParameterValues;
    }


    private boolean isCompleted(){
        boolean bIsCompleted = true;
        for (int i = 0; i < m_aParameterPanels.length; i++){
            bIsCompleted = m_aParameterPanels[i].isCompleted();
            if (!bIsCompleted){
                break;
            }
        }
        return bIsCompleted;
    }


    private void addBorderPanel(java.awt.Container _jContainer, String _sLayout){
        JPanel jPnlBorder = new JPanel();
        jPnlBorder.setPreferredSize(new java.awt.Dimension(10, 10));
        _jContainer.add(jPnlBorder, _sLayout);
    }

    private void addGapPanel(java.awt.Container _jContainer){
        JPanel jPnlBorder = new JPanel();
        jPnlBorder.setPreferredSize(new java.awt.Dimension(10, 10));
        jPnlBorder.setMaximumSize(new java.awt.Dimension(10, 10));
        _jContainer.add(jPnlBorder);
    }


    private class ParameterPanel extends JPanel{
        private JComponent m_jComponent;
        private TypeClass m_aTypeClass = null;

        public ParameterPanel(ParamInfo _aParamInfo){
            JTextField jTextField = new JTextField();
            JComboBox jComboBox = new JComboBox();
            m_aTypeClass =  _aParamInfo.aType.getTypeClass();
            setLayout(new java.awt.BorderLayout());
            addBorderPanel(this, BorderLayout.NORTH);
            addBorderPanel(this, BorderLayout.SOUTH);
            JPanel jPnlCenter1 = new javax.swing.JPanel();
            jPnlCenter1.setLayout(new javax.swing.BoxLayout(jPnlCenter1, javax.swing.BoxLayout.X_AXIS));
            JLabel jLabel1 = new JLabel();
            jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
            String sParamText = _aParamInfo.aName + " (" + _aParamInfo.aType.getName() +")";
            jLabel1.setText(sParamText);
            jPnlCenter1.add(jLabel1);
            addGapPanel(jPnlCenter1);
            switch (m_aTypeClass.getValue()){
                case TypeClass.BOOLEAN_value:
                    jComboBox.setBackground(new java.awt.Color(255, 255, 255));
                    jComboBox.setPreferredSize(new java.awt.Dimension(50, 19));
                    jComboBox.addItem("True");
                    jComboBox.addItem("False");
                    jComboBox.addKeyListener(new UpdateUIAdapter());
                    jPnlCenter1.add(jComboBox);
                    m_jComponent = jComboBox;
                    break;
                case TypeClass.BYTE_value:
                case TypeClass.CHAR_value:
                case TypeClass.DOUBLE_value:
                case TypeClass.ENUM_value:
                case TypeClass.FLOAT_value:
                case TypeClass.HYPER_value:
                case TypeClass.LONG_value:
                case TypeClass.SHORT_value:
                case TypeClass.STRING_value:
                case TypeClass.UNSIGNED_HYPER_value:
                case TypeClass.UNSIGNED_LONG_value:
                case TypeClass.UNSIGNED_SHORT_value:
                    jTextField.setPreferredSize(new java.awt.Dimension(50, 19));
                    jTextField.addKeyListener(new UpdateUIAdapter());
                    jPnlCenter1.add(jTextField);
                    m_jComponent = jTextField;
                    break;
                default:
                    System.out.println("Type " + m_aTypeClass.getValue() + " not yet defined in 'ParameterPanel()'");
            }
            add(jPnlCenter1, java.awt.BorderLayout.CENTER);
            JPanel jPnlEast = new JPanel();
            add(jPnlEast, BorderLayout.EAST);
        }

        private JComponent getInputComponent(){
            return m_jComponent;
        }

        public Object getValue(){
            Object oReturn = null;
            if (m_jComponent instanceof JTextField){
                String sText = ((JTextField) m_jComponent).getText();
                oReturn = Introspector.getIntrospector().getValueOfText(m_aTypeClass, sText);
            }
            else{
                JComboBox jComboBox = ((JComboBox) m_jComponent);
                oReturn =  Boolean.valueOf(jComboBox.getSelectedIndex() == 0);
            }
            return oReturn;
        }


        public boolean isCompleted(){
            if (m_jComponent instanceof JTextField){
                return !((JTextField) m_jComponent).getText().equals("");
            }
            else{
                return true;
            }
        }
    }


    private class UpdateUIAdapter extends KeyAdapter{
        public void keyReleased(KeyEvent e){
            boolean bIsCompleted = isCompleted();
            jOKButton.setEnabled(bIsCompleted);
            jInvokeButton.setEnabled(bIsCompleted);
            if (jLblResult != null){
                jLblResult.setEnabled(false);
                jLblResult.invalidate();
            }
        }
    }



    private class ButtonPanel extends JPanel{
        public ButtonPanel(){
            super();
            setLayout(new BorderLayout());
            addBorderPanel(this, BorderLayout.NORTH);
            addBorderPanel(this, BorderLayout.SOUTH);
            JPanel jPnlBottomCenter = new JPanel();
            jPnlBottomCenter.setLayout(new javax.swing.BoxLayout(jPnlBottomCenter, javax.swing.BoxLayout.X_AXIS));
            jHelpButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    oActionListener.actionPerformed(e);
                }
            });
            jHelpButton.setEnabled(oActionListener != null);
            jPnlBottomCenter.add(jHelpButton);
            addGapPanel(jPnlBottomCenter);
            jOKButton.setEnabled(false);
            jOKButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    invokeParameterMethod();
                    dispose();
                }
            });
            jOKButton.setEnabled(isCompleted());
            jInvokeButton.setEnabled(isCompleted());
            jInvokeButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    invokeParameterMethod();
                }
            });

            jPnlBottomCenter.add(jOKButton);
            addGapPanel(jPnlBottomCenter);
            jPnlBottomCenter.add(jInvokeButton);
            addGapPanel(jPnlBottomCenter);
            JButton jCancelButton = new JButton("Cancel");
            jCancelButton.setFocusCycleRoot(true);
            jCancelButton.setFocusPainted(true);
            jCancelButton.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    bisdiposed = true;
                    dispose();
                }
            });

            jPnlBottomCenter.add(jCancelButton);
            add(jPnlBottomCenter);
        }
    }


    public void addActionListener(ActionListener _oActionListener){
        oActionListener = _oActionListener;
        jHelpButton.setEnabled(oActionListener != null);
    }


    public void invokeParameterMethod(){
    try{
        Object[] oParameters = getParameterValues();
        m_oUnoReturnObject = m_xUnoMethodNode.invoke(m_oUnoObject, oParameters);
        insertResultPanel(null);
    } catch (Exception ex) {
        insertResultPanel(ex);
        m_oUnoReturnObject = null;
    }}

}
