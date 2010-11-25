/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package graphical;

/**
 *
 * @author ll93751
 */
public class Tolerance
{
    private int m_nTolerance;
    public Tolerance(int _nAccept)
    {
        m_nTolerance = _nAccept;
    }
    public int getAccept()
    {
        return m_nTolerance;
    }
}
