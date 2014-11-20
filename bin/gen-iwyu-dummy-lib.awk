BEGIN  { domatch = 0; }

{
    if ($0 ~ /use_external(s)?,/ )
    {
        if (index($0, "))"))
        {
            gsub(/.*,/, "");
            gsub(/\)+/, ""); 
            if (!($0 in exts))
            {
                exts[$0];
                print $0;
            }
        }
        else
        {
           domatch = 1;
        }
    }
    else if ($0 ~ /\)\)/ )
    {
        domatch = 0;
    }
    else if (domatch == 1)
    {
        if (!($1 in exts))
        {
            exts[$1];
            print $1;
        }
    }
}

