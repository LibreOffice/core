package installer;

import java.awt.*;

public class Banner extends Canvas
{
    Image img;
    Banner()
    {
        setBackground(Color.white);
        img = Toolkit.getDefaultToolkit().createImage("sidebar.jpg");
    }

    public void paint(Graphics g)
    {
        g.drawImage(img, 0, 0, Color.white, null);
        g.dispose();
    }

    public void update(Graphics g)
    {
        super.update(g);
    }

    public Dimension getPreferredSize()
    {
        return new Dimension(137, 358);
    }

}
