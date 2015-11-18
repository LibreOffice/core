{
    x++
    y=$0
}

END { 
    tmp = substr(y, 0, index(y, ":")-1)
    if (x==1) print "sed -i '/[[:space:]]" p1 "[[:space:]]/d' " tmp
    # mark these as potential places to inline a constant
    if (x==2) print "#inline " p1 " " tmp
}




# | xargs -P 4 -Ixxx sh -c "git grep -w 'xxx' | awk '{ x++; y=\$0 } END { if (x==1) print y }' && echo \"xxx\" 1>&2"
# 	sed -i '' '/pattern/d'
# | awk  'arr[$0]++ END { for (i in arr) { if(arr[i]==1) print i } }' \
# | awk -f find-unused-defines.awk
