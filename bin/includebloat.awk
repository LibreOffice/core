#!/usr/bin/gawk -f

BEGIN {
    cmd = "find workdir/Dep/CxxObject/ -name *.d | xargs cat"
    while ((cmd | getline) > 0) {
        if ($0 ~ /^ .*\\$/) {
            gsub(/^ /, "");
            gsub(/ *\\$/, "");
            includes[$1]++
            if ($2) {
                # GCC emits 2 per line if short enough!
                includes[$2]++
            }
        }
    }
    exit
}

END {
    for (inc in includes) {
        cmd = "wc -c " inc
        if ((cmd | getline) < 0)
            print "ERROR on: " cmd
        sizes[inc] = $1 # $0 is wc -c output, $1 is size
        totals[inc] = $1 * includes[inc]
        totalsize += totals[inc]
        close(cmd)
    }
    PROCINFO["sorted_in"] = "@val_num_desc"
    print "sum total bytes included: " totalsize
    for (inc in totals) {
        print totals[inc], sizes[inc], includes[inc], inc
    }
}
